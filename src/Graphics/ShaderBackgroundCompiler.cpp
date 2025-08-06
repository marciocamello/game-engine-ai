#include "Graphics/ShaderBackgroundCompiler.h"
#include "Graphics/Shader.h"
#include "Graphics/ShaderManager.h"
#include "Core/Logger.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <sstream>

namespace GameEngine {
    ShaderBackgroundCompiler& ShaderBackgroundCompiler::GetInstance() {
        static ShaderBackgroundCompiler instance;
        return instance;
    }

    bool ShaderBackgroundCompiler::Initialize() {
        if (m_initialized.load()) {
            return true;
        }

        m_shutdown.store(false);
        m_paused.store(false);
        
        // Start worker threads
        StartWorkerThreads();
        
        m_initialized.store(true);
        LOG_INFO("ShaderBackgroundCompiler initialized with " + std::to_string(m_maxWorkerThreads) + " worker threads");
        
        return true;
    }

    void ShaderBackgroundCompiler::Shutdown() {
        if (!m_initialized.load()) {
            return;
        }

        m_shutdown.store(true);
        
        // Stop progressive loading
        StopProgressiveLoading();
        
        // Cancel all pending jobs
        CancelAllJobs();
        
        // Stop worker threads
        StopWorkerThreads();
        
        // Clear cache
        ClearCompilationCache();
        
        m_initialized.store(false);
        LOG_INFO("ShaderBackgroundCompiler shutdown complete");
    }

    std::future<std::shared_ptr<Shader>> ShaderBackgroundCompiler::SubmitCompilationJob(
        const std::string& name,
        const std::string& vertexSource,
        const std::string& fragmentSource,
        const std::string& geometrySource,
        const std::string& computeSource,
        const ShaderVariant& variant,
        int priority,
        std::function<void(std::shared_ptr<Shader>)> callback) {
        
        if (!m_initialized.load()) {
            // Return a failed future
            std::promise<std::shared_ptr<Shader>> promise;
            auto future = promise.get_future();
            promise.set_value(nullptr);
            return future;
        }

        // Check cache first
        std::string cacheKey = name + "_" + variant.GenerateHash();
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            auto it = m_compilationCache.find(cacheKey);
            if (it != m_compilationCache.end()) {
                std::promise<std::shared_ptr<Shader>> promise;
                auto future = promise.get_future();
                promise.set_value(it->second);
                if (callback) {
                    callback(it->second);
                }
                return future;
            }
        }

        // Store shader sources for potential variant compilation
        {
            std::lock_guard<std::mutex> lock(m_sourcesMutex);
            m_shaderSources[name] = {vertexSource, fragmentSource, geometrySource, computeSource};
        }

        // Create compilation job
        auto job = std::make_unique<ShaderCompilationJob>();
        job->name = name;
        job->vertexSource = vertexSource;
        job->fragmentSource = fragmentSource;
        job->geometrySource = geometrySource;
        job->computeSource = computeSource;
        job->variant = variant;
        job->priority = priority;
        job->callback = callback;

        auto future = job->promise.get_future();

        // Add to queue
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_jobQueue.push(std::move(job));
            m_stats.totalJobsSubmitted++;
        }
        
        m_queueCondition.notify_one();
        
        return future;
    }

    std::future<std::shared_ptr<Shader>> ShaderBackgroundCompiler::SubmitVariantCompilationJob(
        const std::string& baseName,
        const ShaderVariant& variant,
        int priority,
        std::function<void(std::shared_ptr<Shader>)> callback) {
        
        // Get base shader sources
        ShaderSources sources;
        {
            std::lock_guard<std::mutex> lock(m_sourcesMutex);
            auto it = m_shaderSources.find(baseName);
            if (it == m_shaderSources.end()) {
                // Return failed future if base shader sources not found
                std::promise<std::shared_ptr<Shader>> promise;
                auto future = promise.get_future();
                promise.set_value(nullptr);
                return future;
            }
            sources = it->second;
        }

        std::string variantName = baseName + "_" + variant.GenerateHash();
        
        return SubmitCompilationJob(
            variantName,
            sources.vertexSource,
            sources.fragmentSource,
            sources.geometrySource,
            sources.computeSource,
            variant,
            priority,
            callback
        );
    }

    void ShaderBackgroundCompiler::StartProgressiveLoading(const std::vector<std::string>& shaderPaths) {
        if (m_progressiveLoadingActive.load()) {
            StopProgressiveLoading();
        }

        {
            std::lock_guard<std::mutex> lock(m_progressiveLoadingMutex);
            m_progressiveLoadingQueue = shaderPaths;
            m_progressiveLoadingIndex = 0;
        }

        m_progressiveLoadingActive.store(true);
        m_progressiveLoadingThread = std::thread(&ShaderBackgroundCompiler::ProgressiveLoadingThreadFunction, this);
        
        LOG_INFO("Started progressive loading of " + std::to_string(shaderPaths.size()) + " shaders");
    }

    void ShaderBackgroundCompiler::StopProgressiveLoading() {
        if (!m_progressiveLoadingActive.load()) {
            return;
        }

        m_progressiveLoadingActive.store(false);
        
        if (m_progressiveLoadingThread.joinable()) {
            m_progressiveLoadingThread.join();
        }

        LOG_INFO("Stopped progressive loading");
    }

    void ShaderBackgroundCompiler::PrecompileVariants(const std::string& baseName, const std::vector<ShaderVariant>& variants) {
        if (!m_initialized.load()) {
            return;
        }

        // Submit variant compilation jobs with lower priority
        for (const auto& variant : variants) {
            SubmitVariantCompilationJob(baseName, variant, -1); // Low priority
        }

        LOG_INFO("Submitted " + std::to_string(variants.size()) + " variant compilation jobs for " + baseName);
    }

    void ShaderBackgroundCompiler::PrecompileCommonVariants() {
        // Define common shader variants
        std::vector<ShaderVariant> commonVariants;
        
        // Basic variants
        ShaderVariant basic;
        commonVariants.push_back(basic);
        
        // PBR variants
        ShaderVariant pbrBasic;
        pbrBasic.AddDefine("USE_PBR", "1");
        commonVariants.push_back(pbrBasic);
        
        ShaderVariant pbrWithNormalMap;
        pbrWithNormalMap.AddDefine("USE_PBR", "1");
        pbrWithNormalMap.AddDefine("USE_NORMAL_MAP", "1");
        commonVariants.push_back(pbrWithNormalMap);
        
        ShaderVariant pbrWithEmission;
        pbrWithEmission.AddDefine("USE_PBR", "1");
        pbrWithEmission.AddDefine("USE_EMISSION_MAP", "1");
        commonVariants.push_back(pbrWithEmission);

        // Get all registered shaders and precompile variants
        auto shaderNames = ShaderManager::GetInstance().GetShaderNames();
        for (const auto& shaderName : shaderNames) {
            PrecompileVariants(shaderName, commonVariants);
        }

        LOG_INFO("Started precompilation of common variants for " + std::to_string(shaderNames.size()) + " shaders");
    }

    void ShaderBackgroundCompiler::SetMaxWorkerThreads(size_t count) {
        if (count == 0) count = 1;
        if (count > std::thread::hardware_concurrency()) {
            count = std::thread::hardware_concurrency();
        }

        if (count != m_maxWorkerThreads) {
            bool wasInitialized = m_initialized.load();
            if (wasInitialized) {
                StopWorkerThreads();
            }
            
            m_maxWorkerThreads = count;
            
            if (wasInitialized) {
                StartWorkerThreads();
            }
            
            LOG_INFO("Set worker thread count to " + std::to_string(count));
        }
    }

    void ShaderBackgroundCompiler::PauseCompilation() {
        m_paused.store(true);
        LOG_INFO("Paused background shader compilation");
    }

    void ShaderBackgroundCompiler::ResumeCompilation() {
        m_paused.store(false);
        m_queueCondition.notify_all();
        LOG_INFO("Resumed background shader compilation");
    }

    ShaderBackgroundCompiler::CompilationStats ShaderBackgroundCompiler::GetStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        CompilationStats stats = m_stats;
        
        // Update current queue size
        {
            std::lock_guard<std::mutex> queueLock(m_queueMutex);
            stats.currentQueueSize = m_jobQueue.size();
        }
        
        return stats;
    }

    void ShaderBackgroundCompiler::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = CompilationStats{};
        LOG_INFO("Reset compilation statistics");
    }

    void ShaderBackgroundCompiler::SetJobPriority(const std::string& name, int priority) {
        std::lock_guard<std::mutex> lock(m_activeJobsMutex);
        auto it = m_activeJobs.find(name);
        if (it != m_activeJobs.end()) {
            it->second->priority = priority;
        }
    }

    void ShaderBackgroundCompiler::CancelJob(const std::string& name) {
        std::lock_guard<std::mutex> lock(m_activeJobsMutex);
        auto it = m_activeJobs.find(name);
        if (it != m_activeJobs.end()) {
            it->second->promise.set_value(nullptr);
            m_activeJobs.erase(it);
        }
    }

    void ShaderBackgroundCompiler::CancelAllJobs() {
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            while (!m_jobQueue.empty()) {
                auto job = std::move(const_cast<std::unique_ptr<ShaderCompilationJob>&>(m_jobQueue.top()));
                m_jobQueue.pop();
                job->promise.set_value(nullptr);
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(m_activeJobsMutex);
            for (auto& pair : m_activeJobs) {
                pair.second->promise.set_value(nullptr);
            }
            m_activeJobs.clear();
        }
        
        LOG_INFO("Cancelled all pending compilation jobs");
    }

    void ShaderBackgroundCompiler::SetMaxCacheSize(size_t maxSizeBytes) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_maxCacheSize = maxSizeBytes;
        
        // Evict entries if we're over the new limit
        if (m_currentCacheSize > m_maxCacheSize) {
            EvictOldestCacheEntries();
        }
    }

    void ShaderBackgroundCompiler::ClearCompilationCache() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        m_compilationCache.clear();
        m_currentCacheSize = 0;
        LOG_INFO("Cleared shader compilation cache");
    }

    size_t ShaderBackgroundCompiler::GetCacheSize() const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_currentCacheSize;
    }

    // Private methods
    void ShaderBackgroundCompiler::WorkerThreadFunction() {
        while (!m_shutdown.load()) {
            std::unique_ptr<ShaderCompilationJob> job;
            
            // Wait for job or shutdown
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCondition.wait(lock, [this] {
                    return m_shutdown.load() || (!m_jobQueue.empty() && !m_paused.load());
                });
                
                if (m_shutdown.load()) {
                    break;
                }
                
                if (!m_jobQueue.empty()) {
                    job = std::move(const_cast<std::unique_ptr<ShaderCompilationJob>&>(m_jobQueue.top()));
                    m_jobQueue.pop();
                }
            }
            
            if (job) {
                // Track active job
                {
                    std::lock_guard<std::mutex> lock(m_activeJobsMutex);
                    m_activeJobs[job->name] = std::move(job);
                }
                
                // Get job reference (it's now in active jobs)
                ShaderCompilationJob* jobPtr;
                {
                    std::lock_guard<std::mutex> lock(m_activeJobsMutex);
                    jobPtr = m_activeJobs[job->name].get();
                }
                
                // Compile shader
                auto startTime = std::chrono::high_resolution_clock::now();
                auto shader = CompileShaderJob(*jobPtr);
                auto endTime = std::chrono::high_resolution_clock::now();
                
                // Update statistics
                {
                    std::lock_guard<std::mutex> lock(m_statsMutex);
                    if (shader) {
                        m_stats.totalJobsCompleted++;
                    } else {
                        m_stats.totalJobsFailed++;
                    }
                    
                    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
                    double compilationTimeMs = duration.count() / 1000.0;
                    m_stats.totalCompilationTime += compilationTimeMs;
                    m_stats.averageCompilationTime = m_stats.totalCompilationTime / 
                        (m_stats.totalJobsCompleted + m_stats.totalJobsFailed);
                }
                
                // Remove from active jobs and process completion
                {
                    std::lock_guard<std::mutex> lock(m_activeJobsMutex);
                    auto activeJob = std::move(m_activeJobs[jobPtr->name]);
                    m_activeJobs.erase(jobPtr->name);
                    ProcessCompletedJob(std::move(activeJob), shader);
                }
            }
        }
    }

    void ShaderBackgroundCompiler::StartWorkerThreads() {
        StopWorkerThreads(); // Ensure no existing threads
        
        m_workerThreads.reserve(m_maxWorkerThreads);
        for (size_t i = 0; i < m_maxWorkerThreads; ++i) {
            m_workerThreads.emplace_back(&ShaderBackgroundCompiler::WorkerThreadFunction, this);
        }
    }

    void ShaderBackgroundCompiler::StopWorkerThreads() {
        m_queueCondition.notify_all();
        
        for (auto& thread : m_workerThreads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        m_workerThreads.clear();
    }

    std::shared_ptr<Shader> ShaderBackgroundCompiler::CompileShaderJob(const ShaderCompilationJob& job) {
        try {
            auto shader = std::make_shared<Shader>();
            
            // Apply variant defines to shader sources
            std::string vertexSource = job.vertexSource;
            std::string fragmentSource = job.fragmentSource;
            
            // Add variant defines to the beginning of each shader
            std::string defines;
            for (const auto& define : job.variant.defines) {
                defines += "#define " + define.first;
                if (!define.second.empty() && define.second != "1") {
                    defines += " " + define.second;
                }
                defines += "\n";
            }
            
            if (!defines.empty()) {
                // Insert defines after #version directive
                auto insertDefines = [&defines](std::string& source) {
                    size_t versionPos = source.find("#version");
                    if (versionPos != std::string::npos) {
                        size_t lineEnd = source.find('\n', versionPos);
                        if (lineEnd != std::string::npos) {
                            source.insert(lineEnd + 1, defines);
                        }
                    } else {
                        source = defines + source;
                    }
                };
                
                insertDefines(vertexSource);
                insertDefines(fragmentSource);
            }
            
            // Compile shader
            bool success = false;
            if (!job.computeSource.empty()) {
                success = shader->CompileFromSource(job.computeSource, Shader::Type::Compute) &&
                         shader->LinkProgram();
            } else {
                success = shader->LoadFromSource(vertexSource, fragmentSource);
            }
            
            if (success) {
                // Cache the compiled shader
                std::string cacheKey = job.name + "_" + job.variant.GenerateHash();
                {
                    std::lock_guard<std::mutex> lock(m_cacheMutex);
                    m_compilationCache[cacheKey] = shader;
                    UpdateCacheSize();
                }
                
                return shader;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Background shader compilation failed for " + job.name + ": " + e.what());
        }
        
        return nullptr;
    }

    void ShaderBackgroundCompiler::ProcessCompletedJob(std::unique_ptr<ShaderCompilationJob> job, std::shared_ptr<Shader> shader) {
        // Set promise result
        job->promise.set_value(shader);
        
        // Call callback if provided
        if (job->callback) {
            try {
                job->callback(shader);
            } catch (const std::exception& e) {
                LOG_ERROR("Shader compilation callback failed: " + std::string(e.what()));
            }
        }
        
        if (shader) {
            LOG_INFO("Background compilation completed for shader: " + job->name);
        } else {
            LOG_WARNING("Background compilation failed for shader: " + job->name);
        }
    }

    void ShaderBackgroundCompiler::ProgressiveLoadingThreadFunction() {
        while (m_progressiveLoadingActive.load() && !m_shutdown.load()) {
            LoadNextShaderBatch();
            
            // Wait between batches to avoid overwhelming the system
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void ShaderBackgroundCompiler::LoadNextShaderBatch() {
        std::vector<std::string> batch;
        
        {
            std::lock_guard<std::mutex> lock(m_progressiveLoadingMutex);
            size_t remaining = m_progressiveLoadingQueue.size() - m_progressiveLoadingIndex;
            if (remaining == 0) {
                m_progressiveLoadingActive.store(false);
                return;
            }
            
            size_t batchSize = std::min(m_progressiveLoadingBatchSize, remaining);
            batch.reserve(batchSize);
            
            for (size_t i = 0; i < batchSize; ++i) {
                batch.push_back(m_progressiveLoadingQueue[m_progressiveLoadingIndex + i]);
            }
            
            m_progressiveLoadingIndex += batchSize;
        }
        
        // Submit compilation jobs for this batch
        for (const auto& shaderPath : batch) {
            // Load shader source from file
            std::ifstream file(shaderPath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                std::string source = buffer.str();
                
                // Determine shader type and submit job
                std::string name = shaderPath.substr(shaderPath.find_last_of("/\\") + 1);
                if (shaderPath.find(".vert") != std::string::npos) {
                    // Vertex shader - need to find corresponding fragment shader
                    std::string fragPath = shaderPath;
                    fragPath.replace(fragPath.find(".vert"), 5, ".frag");
                    
                    std::ifstream fragFile(fragPath);
                    if (fragFile.is_open()) {
                        std::stringstream fragBuffer;
                        fragBuffer << fragFile.rdbuf();
                        std::string fragSource = fragBuffer.str();
                        
                        SubmitCompilationJob(name, source, fragSource, "", "", ShaderVariant{}, -2); // Very low priority
                    }
                }
            }
        }
    }

    void ShaderBackgroundCompiler::UpdateCacheSize() {
        // Simple estimation - in a real implementation, you'd calculate actual memory usage
        m_currentCacheSize = m_compilationCache.size() * 1024; // Rough estimate
        
        if (m_currentCacheSize > m_maxCacheSize) {
            EvictOldestCacheEntries();
        }
    }

    void ShaderBackgroundCompiler::EvictOldestCacheEntries() {
        // Simple LRU eviction - remove half the entries
        // In a real implementation, you'd track access times
        size_t targetSize = m_maxCacheSize / 2;
        size_t toRemove = m_compilationCache.size() - (targetSize / 1024);
        
        auto it = m_compilationCache.begin();
        for (size_t i = 0; i < toRemove && it != m_compilationCache.end(); ++i) {
            it = m_compilationCache.erase(it);
        }
        
        UpdateCacheSize();
    }
}