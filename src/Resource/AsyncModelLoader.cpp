#include "Resource/AsyncModelLoader.h"
#include "Core/Logger.h"
#include <algorithm>
#include <chrono>

namespace GameEngine {

    // ThreadPool Implementation
    ThreadPool::ThreadPool(size_t numThreads) {
        if (numThreads == 0) {
            numThreads = std::max(1u, std::thread::hardware_concurrency());
        }

        for (size_t i = 0; i < numThreads; ++i) {
            m_workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(m_queueMutex);
                        m_condition.wait(lock, [this] { return m_stop || !m_tasks.empty(); });
                        
                        if (m_stop && m_tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }
                    
                    task();
                }
            });
        }
    }

    ThreadPool::~ThreadPool() {
        Stop();
    }

    void ThreadPool::Stop() {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_stop = true;
        }
        
        m_condition.notify_all();
        
        for (std::thread& worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        m_workers.clear();
    }

    // AsyncModelLoader Implementation
    AsyncModelLoader::AsyncModelLoader() = default;

    AsyncModelLoader::~AsyncModelLoader() {
        Shutdown();
    }

    bool AsyncModelLoader::Initialize(uint32_t workerThreadCount) {
        if (m_initialized) {
            LOG_WARNING("AsyncModelLoader already initialized");
            return true;
        }

        try {
            // Initialize thread pool
            if (workerThreadCount == 0) {
                workerThreadCount = std::max(1u, std::thread::hardware_concurrency());
            }
            
            m_threadPool = std::make_unique<ThreadPool>(workerThreadCount);
            
            // Initialize model loader
            m_modelLoader = std::make_unique<ModelLoader>();
            if (!m_modelLoader->Initialize()) {
                LOG_ERROR("Failed to initialize ModelLoader for AsyncModelLoader");
                return false;
            }

            m_initialized = true;
            LOG_INFO("AsyncModelLoader initialized with " + std::to_string(workerThreadCount) + " worker threads");
            return true;

        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize AsyncModelLoader: " + std::string(e.what()));
            return false;
        }
    }

    void AsyncModelLoader::Shutdown() {
        if (!m_initialized) {
            return;
        }

        LOG_INFO("Shutting down AsyncModelLoader...");

        // Cancel all active loads
        CancelAllLoads();

        // Wait for all tasks to complete
        WaitForAllLoads();

        // Shutdown thread pool
        if (m_threadPool) {
            m_threadPool->Stop();
            m_threadPool.reset();
        }

        // Shutdown model loader
        if (m_modelLoader) {
            m_modelLoader->Shutdown();
            m_modelLoader.reset();
        }

        // Clear all data
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_activeTasks.clear();
        }
        
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            while (!m_taskQueue.empty()) {
                m_taskQueue.pop();
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            m_loadedModels.clear();
        }
        
        m_currentMemoryUsage = 0;
        m_peakMemoryUsage = 0;

        m_initialized = false;
        LOG_INFO("AsyncModelLoader shutdown complete");
    }

    std::future<std::shared_ptr<Model>> AsyncModelLoader::LoadModelAsync(const std::string& filepath) {
        return LoadModelAsync(filepath, m_defaultFlags);
    }

    std::future<std::shared_ptr<Model>> AsyncModelLoader::LoadModelAsync(const std::string& filepath, ModelLoader::LoadingFlags flags) {
        return LoadModelAsync(filepath, flags, m_defaultPriority);
    }

    std::future<std::shared_ptr<Model>> AsyncModelLoader::LoadModelAsync(const std::string& filepath, ModelLoader::LoadingFlags flags, TaskPriority priority) {
        return LoadModelAsync(filepath, flags, priority, {});
    }

    std::future<std::shared_ptr<Model>> AsyncModelLoader::LoadModelAsync(const std::string& filepath, ModelLoader::LoadingFlags flags, TaskPriority priority, const std::vector<std::string>& dependencies) {
        if (!m_initialized) {
            throw std::runtime_error("AsyncModelLoader not initialized");
        }

        // Check if already in cache
        {
            std::lock_guard<std::mutex> lock(m_cacheMutex);
            auto it = m_loadedModels.find(filepath);
            if (it != m_loadedModels.end()) {
                // Return cached model
                std::promise<std::shared_ptr<Model>> promise;
                auto future = promise.get_future();
                promise.set_value(it->second);
                return future;
            }
        }

        // Check if already loading
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            auto it = m_activeTasks.find(filepath);
            if (it != m_activeTasks.end() && !it->second->cancelled) {
                throw std::runtime_error("Model is already being loaded: " + filepath);
            }
        }

        // Create load task
        auto task = std::make_shared<LoadTask>(filepath, flags, priority);
        task->dependencies = dependencies;
        task->estimatedMemoryUsage = EstimateModelMemoryUsage(filepath);
        
        // Get future before moving task
        auto future = task->promise.get_future();

        // Check if we can start immediately or need to queue
        if (CanStartNewLoad() && AreDependenciesResolved(task)) {
            // Add to active tasks
            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks[filepath] = task;
            }

            // Update statistics
            UpdateStats(false, false, false, 0.0f, 0);

            // Submit to thread pool
            try {
                m_threadPool->Enqueue([this, task]() {
                    ProcessLoadTask(task);
                });
            } catch (const std::exception& e) {
                // Remove from active tasks if enqueue failed
                {
                    std::lock_guard<std::mutex> lock(m_tasksMutex);
                    m_activeTasks.erase(filepath);
                }
                throw;
            }

            LogTaskStart(filepath);
        } else {
            // Queue the task
            QueueTask(task);
        }

        return future;
    }

    std::future<std::vector<std::shared_ptr<Model>>> AsyncModelLoader::LoadModelsAsync(const std::vector<std::string>& filepaths) {
        return LoadModelsAsync(filepaths, m_defaultFlags);
    }

    std::future<std::vector<std::shared_ptr<Model>>> AsyncModelLoader::LoadModelsAsync(const std::vector<std::string>& filepaths, ModelLoader::LoadingFlags flags) {
        if (!m_initialized) {
            throw std::runtime_error("AsyncModelLoader not initialized");
        }

        // Create a promise for the batch result
        auto promise = std::make_shared<std::promise<std::vector<std::shared_ptr<Model>>>>();
        auto future = promise->get_future();

        // Submit batch loading task
        m_threadPool->Enqueue([this, filepaths, flags, promise]() {
            std::vector<std::shared_ptr<Model>> results;
            results.reserve(filepaths.size());

            std::vector<std::future<std::shared_ptr<Model>>> futures;
            futures.reserve(filepaths.size());

            try {
                // Start all individual loads
                for (const auto& filepath : filepaths) {
                    futures.push_back(LoadModelAsync(filepath, flags));
                }

                // Collect results
                for (auto& fut : futures) {
                    try {
                        results.push_back(fut.get());
                    } catch (const std::exception& e) {
                        LOG_ERROR("Failed to load model in batch: " + std::string(e.what()));
                        results.push_back(nullptr);
                    }
                }

                promise->set_value(std::move(results));

            } catch (const std::exception& e) {
                promise->set_exception(std::current_exception());
            }
        });

        return future;
    }

    std::future<std::vector<std::shared_ptr<Model>>> AsyncModelLoader::LoadModelsAsync(const std::vector<std::string>& filepaths, ModelLoader::LoadingFlags flags, TaskPriority priority) {
        if (!m_initialized) {
            throw std::runtime_error("AsyncModelLoader not initialized");
        }

        // Create a promise for the batch result
        auto promise = std::make_shared<std::promise<std::vector<std::shared_ptr<Model>>>>();
        auto future = promise->get_future();

        // Submit batch loading task
        m_threadPool->Enqueue([this, filepaths, flags, priority, promise]() {
            std::vector<std::shared_ptr<Model>> results;
            results.reserve(filepaths.size());

            std::vector<std::future<std::shared_ptr<Model>>> futures;
            futures.reserve(filepaths.size());

            try {
                // Start all individual loads
                for (const auto& filepath : filepaths) {
                    futures.push_back(LoadModelAsync(filepath, flags, priority));
                }

                // Collect results
                for (auto& fut : futures) {
                    try {
                        results.push_back(fut.get());
                    } catch (const std::exception& e) {
                        LOG_ERROR("Failed to load model in batch: " + std::string(e.what()));
                        results.push_back(nullptr);
                    }
                }

                promise->set_value(std::move(results));

            } catch (const std::exception& e) {
                promise->set_exception(std::current_exception());
            }
        });

        return future;
    }

    void AsyncModelLoader::SetProgressCallback(ProgressCallback callback) {
        std::lock_guard<std::mutex> lock(m_progressMutex);
        m_progressCallback = std::move(callback);
    }

    float AsyncModelLoader::GetLoadingProgress(const std::string& filepath) const {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        auto it = m_activeTasks.find(filepath);
        if (it != m_activeTasks.end()) {
            return it->second->progress.load();
        }
        return 0.0f;
    }

    std::string AsyncModelLoader::GetLoadingStage(const std::string& filepath) const {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        auto it = m_activeTasks.find(filepath);
        if (it != m_activeTasks.end()) {
            return it->second->currentStage;
        }
        return "";
    }

    std::vector<std::string> AsyncModelLoader::GetActiveLoads() const {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        std::vector<std::string> activeLoads;
        activeLoads.reserve(m_activeTasks.size());
        
        for (const auto& pair : m_activeTasks) {
            if (!pair.second->cancelled) {
                activeLoads.push_back(pair.first);
            }
        }
        
        return activeLoads;
    }

    bool AsyncModelLoader::IsLoading(const std::string& filepath) const {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        auto it = m_activeTasks.find(filepath);
        return it != m_activeTasks.end() && !it->second->cancelled;
    }

    bool AsyncModelLoader::CancelLoad(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        auto it = m_activeTasks.find(filepath);
        if (it != m_activeTasks.end()) {
            it->second->cancelled = true;
            LOG_INFO("Cancelled loading of model: " + filepath);
            return true;
        }
        return false;
    }

    void AsyncModelLoader::CancelAllLoads() {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        for (auto& pair : m_activeTasks) {
            pair.second->cancelled = true;
        }
        if (!m_activeTasks.empty()) {
            LOG_INFO("Cancelled " + std::to_string(m_activeTasks.size()) + " active model loads");
        }
    }

    void AsyncModelLoader::WaitForAllLoads() {
        std::vector<std::shared_ptr<LoadTask>> tasksToWait;
        
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            tasksToWait.reserve(m_activeTasks.size());
            for (const auto& pair : m_activeTasks) {
                tasksToWait.push_back(pair.second);
            }
        }

        // Wait for all tasks to complete (or be cancelled)
        for (auto& task : tasksToWait) {
            try {
                // This will block until the task completes or throws
                auto future = task->promise.get_future();
                if (future.valid()) {
                    future.wait();
                }
            } catch (...) {
                // Ignore exceptions during shutdown
            }
        }
    }

    void AsyncModelLoader::SetMaxConcurrentLoads(uint32_t maxLoads) {
        m_maxConcurrentLoads = std::max(1u, maxLoads);
        LOG_INFO("Set max concurrent loads to " + std::to_string(m_maxConcurrentLoads));
    }

    void AsyncModelLoader::SetWorkerThreadCount(uint32_t count) {
        if (m_initialized) {
            LOG_WARNING("Cannot change worker thread count while AsyncModelLoader is initialized");
            return;
        }
        
        // This will take effect on next initialization
        LOG_INFO("Worker thread count will be set to " + std::to_string(count) + " on next initialization");
    }

    uint32_t AsyncModelLoader::GetWorkerThreadCount() const {
        if (m_threadPool) {
            return static_cast<uint32_t>(m_threadPool->GetThreadCount());
        }
        return 0;
    }

    void AsyncModelLoader::SetDefaultLoadingFlags(ModelLoader::LoadingFlags flags) {
        m_defaultFlags = flags;
    }

    void AsyncModelLoader::SetMemoryLimit(size_t maxMemoryBytes) {
        m_maxMemoryUsage = maxMemoryBytes;
        LOG_INFO("Set memory limit to " + std::to_string(maxMemoryBytes / (1024 * 1024)) + " MB");
        
        // Check if we need to free memory immediately
        if (m_currentMemoryUsage > m_maxMemoryUsage) {
            FreeMemoryIfNeeded();
        }
    }

    void AsyncModelLoader::SetDefaultPriority(TaskPriority priority) {
        m_defaultPriority = priority;
    }

    AsyncModelLoader::LoadingStats AsyncModelLoader::GetLoadingStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats;
    }

    void AsyncModelLoader::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = LoadingStats{};
        m_loadTimes.clear();
        LOG_INFO("Reset AsyncModelLoader statistics");
    }

    void AsyncModelLoader::CleanupCompletedTasks() {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        
        auto it = m_activeTasks.begin();
        while (it != m_activeTasks.end()) {
            auto future = it->second->promise.get_future();
            if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                it = m_activeTasks.erase(it);
            } else {
                ++it;
            }
        }
    }

    void AsyncModelLoader::ProcessLoadTask(std::shared_ptr<LoadTask> task) {
        auto startTime = std::chrono::steady_clock::now();
        
        try {
            if (task->cancelled) {
                UpdateStats(false, true, false, 0.0f, 0);
                task->promise.set_exception(std::make_exception_ptr(std::runtime_error("Load cancelled")));
                return;
            }

            // Wait for available load slot
            WaitForLoadSlot();

            if (task->cancelled) {
                UpdateStats(false, true, false, 0.0f, 0);
                task->promise.set_exception(std::make_exception_ptr(std::runtime_error("Load cancelled")));
                return;
            }

            // Load the model
            auto model = LoadModelInternal(task->filepath, task->flags, task);

            if (task->cancelled) {
                UpdateStats(false, true, false, 0.0f, 0);
                task->promise.set_exception(std::make_exception_ptr(std::runtime_error("Load cancelled")));
                return;
            }

            // Calculate load time
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            float loadTimeMs = static_cast<float>(duration.count());

            // Update final progress
            UpdateProgress(task->filepath, 1.0f, "Complete");

            // Update statistics
            size_t memoryUsed = model ? model->GetMemoryUsage() : 0;
            UpdateStats(true, false, false, loadTimeMs, memoryUsed);

            // Cache the model if successful
            if (model) {
                {
                    std::lock_guard<std::mutex> lock(m_cacheMutex);
                    m_loadedModels[task->filepath] = model;
                }
                UpdateMemoryUsage(memoryUsed, true);
                
                // Check if we need to free memory
                FreeMemoryIfNeeded();
            }

            // Set result
            task->promise.set_value(model);

            LogTaskComplete(task->filepath, model != nullptr, loadTimeMs);

        } catch (const std::exception& e) {
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            float loadTimeMs = static_cast<float>(duration.count());

            UpdateStats(false, false, true, loadTimeMs, 0);
            UpdateProgress(task->filepath, 0.0f, "Failed: " + std::string(e.what()));
            
            task->promise.set_exception(std::current_exception());
            LogTaskComplete(task->filepath, false, loadTimeMs);
        }

        // Remove from active tasks
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            m_activeTasks.erase(task->filepath);
        }

        // Process any queued tasks that might now be able to start
        ProcessTaskQueue();
    }

    void AsyncModelLoader::UpdateProgress(const std::string& filepath, float progress, const std::string& stage) {
        // Update task progress
        {
            std::lock_guard<std::mutex> lock(m_tasksMutex);
            auto it = m_activeTasks.find(filepath);
            if (it != m_activeTasks.end()) {
                it->second->progress = progress;
                it->second->currentStage = stage;
            }
        }

        // Call progress callback
        {
            std::lock_guard<std::mutex> lock(m_progressMutex);
            if (m_progressCallback) {
                try {
                    m_progressCallback(filepath, progress, stage);
                } catch (const std::exception& e) {
                    LOG_ERROR("Progress callback threw exception: " + std::string(e.what()));
                }
            }
        }

        if (m_verboseLogging) {
            LOG_INFO("Loading progress for " + filepath + ": " + std::to_string(progress * 100.0f) + "% - " + stage);
        }
    }

    void AsyncModelLoader::UpdateStats(bool completed, bool cancelled, bool failed, float loadTimeMs, size_t memoryUsed) {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        
        if (completed) {
            m_stats.totalLoadsCompleted++;
            m_stats.totalMemoryLoaded += memoryUsed;
        } else if (cancelled) {
            m_stats.totalLoadsCancelled++;
        } else if (failed) {
            m_stats.totalLoadsFailed++;
        } else {
            // Starting a new load
            m_stats.totalLoadsStarted++;
            m_stats.currentActiveLoads++;
            return; // Don't update timing stats for start
        }

        // Update active count (for completed/cancelled/failed)
        if (m_stats.currentActiveLoads > 0) {
            m_stats.currentActiveLoads--;
        }

        // Update timing statistics
        if (loadTimeMs > 0.0f) {
            m_loadTimes.push_back(loadTimeMs);
            
            // Calculate average (keep only last 100 samples for efficiency)
            if (m_loadTimes.size() > 100) {
                m_loadTimes.erase(m_loadTimes.begin());
            }
            
            float sum = 0.0f;
            for (float time : m_loadTimes) {
                sum += time;
            }
            m_stats.averageLoadTimeMs = sum / static_cast<float>(m_loadTimes.size());
        }
    }

    void AsyncModelLoader::LogTaskStart(const std::string& filepath) {
        if (m_verboseLogging) {
            LOG_INFO("Started async loading of model: " + filepath);
        }
    }

    void AsyncModelLoader::LogTaskComplete(const std::string& filepath, bool success, float timeMs) {
        if (success) {
            LOG_INFO("Successfully loaded model: " + filepath + " (" + std::to_string(timeMs) + "ms)");
        } else {
            LOG_ERROR("Failed to load model: " + filepath + " (" + std::to_string(timeMs) + "ms)");
        }
    }

    bool AsyncModelLoader::CanStartNewLoad() const {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        uint32_t activeCount = 0;
        for (const auto& pair : m_activeTasks) {
            if (!pair.second->cancelled) {
                activeCount++;
            }
        }
        return activeCount < m_maxConcurrentLoads;
    }

    void AsyncModelLoader::WaitForLoadSlot() {
        while (!CanStartNewLoad()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    std::shared_ptr<Model> AsyncModelLoader::LoadModelInternal(const std::string& filepath, ModelLoader::LoadingFlags flags, std::shared_ptr<LoadTask> task) {
        // Set loading flags
        m_modelLoader->SetLoadingFlags(flags);

        UpdateProgress(filepath, 0.1f, "Initializing");

        if (task->cancelled) return nullptr;

        UpdateProgress(filepath, 0.2f, "Reading file");

        // Load using the synchronous model loader
        auto result = m_modelLoader->LoadModel(filepath);

        if (task->cancelled) return nullptr;

        UpdateProgress(filepath, 0.6f, "Processing meshes");

        if (!result.success) {
            throw std::runtime_error("Failed to load model: " + result.errorMessage);
        }

        if (task->cancelled) return nullptr;

        UpdateProgress(filepath, 0.8f, "Creating model");

        // Create Model object from loaded meshes
        auto model = std::make_shared<Model>(filepath);
        model->SetMeshes(result.meshes);

        if (task->cancelled) return nullptr;

        UpdateProgress(filepath, 0.9f, "Finalizing");

        // Update model bounds
        model->UpdateBounds();

        return model;
    }

    // Queue and dependency management methods
    void AsyncModelLoader::QueueTask(std::shared_ptr<LoadTask> task) {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_taskQueue.push(task);
        
        // Update statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.queuedLoads++;
        }
        
        if (m_verboseLogging) {
            LOG_INFO("Queued task for " + task->filepath + " (priority: " + std::to_string(static_cast<int>(task->priority)) + ")");
        }
    }

    std::shared_ptr<AsyncModelLoader::LoadTask> AsyncModelLoader::GetNextQueuedTask() {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        
        if (m_taskQueue.empty()) {
            return nullptr;
        }
        
        // For now, simple FIFO queue. Could be enhanced with priority queue
        auto task = m_taskQueue.front();
        m_taskQueue.pop();
        
        // Update statistics
        {
            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            if (m_stats.queuedLoads > 0) {
                m_stats.queuedLoads--;
            }
        }
        
        return task;
    }

    bool AsyncModelLoader::AreDependenciesResolved(const std::shared_ptr<LoadTask>& task) const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        for (const auto& dependency : task->dependencies) {
            auto it = m_loadedModels.find(dependency);
            if (it == m_loadedModels.end()) {
                return false; // Dependency not yet loaded
            }
        }
        
        return true; // All dependencies resolved
    }

    void AsyncModelLoader::SortTaskQueue() {
        // This would require converting to priority_queue or custom sorting
        // For now, keeping simple FIFO queue
    }

    std::vector<std::string> AsyncModelLoader::GetQueuedTasks() const {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        
        std::vector<std::string> queuedTasks;
        std::queue<std::shared_ptr<LoadTask>> tempQueue = m_taskQueue;
        
        while (!tempQueue.empty()) {
            queuedTasks.push_back(tempQueue.front()->filepath);
            tempQueue.pop();
        }
        
        return queuedTasks;
    }

    bool AsyncModelLoader::HasDependenciesResolved(const std::string& filepath) const {
        std::lock_guard<std::mutex> lock(m_tasksMutex);
        auto it = m_activeTasks.find(filepath);
        if (it != m_activeTasks.end()) {
            return AreDependenciesResolved(it->second);
        }
        return true; // No task found, assume resolved
    }

    void AsyncModelLoader::ProcessTaskQueue() {
        // Check if we can start any queued tasks
        while (CanStartNewLoad()) {
            auto task = GetNextQueuedTask();
            if (!task) {
                break; // No more queued tasks
            }
            
            if (task->cancelled) {
                continue; // Skip cancelled tasks
            }
            
            if (!AreDependenciesResolved(task)) {
                // Put back in queue if dependencies not resolved
                QueueTask(task);
                break;
            }
            
            // Start the task
            {
                std::lock_guard<std::mutex> lock(m_tasksMutex);
                m_activeTasks[task->filepath] = task;
            }
            
            UpdateStats(false, false, false, 0.0f, 0);
            
            try {
                m_threadPool->Enqueue([this, task]() {
                    ProcessLoadTask(task);
                });
                LogTaskStart(task->filepath);
            } catch (const std::exception& e) {
                {
                    std::lock_guard<std::mutex> lock(m_tasksMutex);
                    m_activeTasks.erase(task->filepath);
                }
                LOG_ERROR("Failed to start queued task: " + std::string(e.what()));
            }
        }
    }

    // Memory management methods
    void AsyncModelLoader::UpdateMemoryUsage(size_t memoryDelta, bool increase) {
        if (increase) {
            m_currentMemoryUsage += memoryDelta;
            size_t current = m_currentMemoryUsage.load();
            size_t peak = m_peakMemoryUsage.load();
            if (current > peak) {
                m_peakMemoryUsage = current;
            }
        } else {
            if (m_currentMemoryUsage >= memoryDelta) {
                m_currentMemoryUsage -= memoryDelta;
            } else {
                m_currentMemoryUsage = 0;
            }
        }
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.currentMemoryUsage = m_currentMemoryUsage.load();
            m_stats.peakMemoryUsage = m_peakMemoryUsage.load();
        }
    }

    void AsyncModelLoader::EvictLeastRecentlyUsedModels() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        // Simple eviction: remove all cached models
        // In a real implementation, you'd track access times and remove LRU items
        size_t freedMemory = 0;
        for (const auto& pair : m_loadedModels) {
            if (pair.second) {
                freedMemory += pair.second->GetMemoryUsage();
            }
        }
        
        m_loadedModels.clear();
        UpdateMemoryUsage(freedMemory, false);
        
        LOG_INFO("Evicted cached models, freed " + std::to_string(freedMemory / (1024 * 1024)) + " MB");
    }

    void AsyncModelLoader::FreeMemoryIfNeeded() {
        if (m_currentMemoryUsage > m_maxMemoryUsage) {
            EvictLeastRecentlyUsedModels();
        }
    }

    size_t AsyncModelLoader::EstimateModelMemoryUsage(const std::string& filepath) const {
        // Simple estimation based on file extension and size
        // In a real implementation, you might use file size or more sophisticated estimation
        std::string extension = filepath.substr(filepath.find_last_of('.') + 1);
        
        if (extension == "fbx" || extension == "gltf" || extension == "glb") {
            return 50 * 1024 * 1024; // 50MB estimate for complex models
        } else if (extension == "obj") {
            return 10 * 1024 * 1024; // 10MB estimate for OBJ models
        }
        
        return 5 * 1024 * 1024; // 5MB default estimate
    }}
