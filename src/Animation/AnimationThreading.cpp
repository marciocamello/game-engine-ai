#include "Animation/AnimationThreading.h"
#include "Core/Logger.h"
#include <algorithm>
#include <chrono>
#include <thread>

namespace GameEngine {
namespace Animation {

    // AnimationThreadPool implementation
    AnimationThreadPool::AnimationThreadPool() {
        LOG_INFO("AnimationThreadPool created");
    }

    AnimationThreadPool::~AnimationThreadPool() {
        Shutdown();
        LOG_INFO("AnimationThreadPool destroyed");
    }

    bool AnimationThreadPool::Initialize(const AnimationThreadConfig& config) {
        LOG_INFO("Initializing AnimationThreadPool");
        
        m_config = config;
        
        // Auto-detect thread count if not specified
        if (m_config.numThreads == 0) {
            m_config.numThreads = std::max(1u, std::thread::hardware_concurrency() - 1);
        }
        
        // Initialize work stealing queues
        if (m_config.enableWorkStealing) {
            m_workStealingQueues.resize(m_config.numThreads);
            m_workStealingMutexes.resize(m_config.numThreads);
            for (size_t i = 0; i < m_config.numThreads; ++i) {
                m_workStealingMutexes[i] = std::make_unique<std::mutex>();
            }
        }
        
        // Create worker threads
        m_threads.reserve(m_config.numThreads);
        for (size_t i = 0; i < m_config.numThreads; ++i) {
            m_threads.emplace_back(&AnimationThreadPool::WorkerThread, this, i);
            
            // Set CPU affinity if enabled
            if (m_config.enableCPUAffinity) {
                // Platform-specific CPU affinity code would go here
                // For now, just log the intention
                LOG_DEBUG("Thread affinity would be set for thread");
            }
        }
        
        // Initialize statistics
        m_lastStatsUpdate = std::chrono::steady_clock::now();
        
        LOG_INFO("AnimationThreadPool initialized successfully");
        return true;
    }

    void AnimationThreadPool::Shutdown() {
        LOG_INFO("Shutting down AnimationThreadPool");
        
        // Signal shutdown
        m_shutdown = true;
        
        // Wake up all threads
        m_queueCondition.notify_all();
        
        // Wait for all threads to finish
        for (auto& thread : m_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        
        m_threads.clear();
        m_workStealingQueues.clear();
        m_workStealingMutexes.clear();
        
        LOG_INFO("AnimationThreadPool shutdown complete");
    }

    std::future<void> AnimationThreadPool::SubmitTask(AnimationTask task) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();
        
        // Wrap the task to fulfill the promise
        auto wrappedTask = [task = std::move(task), promise]() mutable {
            try {
                task.task();
                promise->set_value();
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };
        
        task.task = std::move(wrappedTask);
        
        // Add to queue
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (m_taskQueue.size() >= m_config.maxQueueSize) {
                LOG_WARNING("Animation thread pool queue is full, dropping task");
                promise->set_exception(std::make_exception_ptr(std::runtime_error("Queue full")));
                return future;
            }
            
            m_taskQueue.push(std::move(task));
            m_stats.totalTasksQueued++;
        }
        
        m_queueCondition.notify_one();
        return future;
    }

    std::future<void> AnimationThreadPool::SubmitTask(std::function<void()> task, AnimationTaskPriority priority) {
        return SubmitTask(AnimationTask(std::move(task), priority));
    }

    std::future<void> AnimationThreadPool::SubmitBatch(const AnimationBatch& batch) {
        auto promise = std::make_shared<std::promise<void>>();
        auto future = promise->get_future();
        
        if (batch.empty()) {
            promise->set_value();
            return future;
        }
        
        // Create a task that processes the entire batch
        auto batchTask = [batch, promise]() {
            try {
                for (size_t i = 0; i < batch.controllers.size(); ++i) {
                    if (batch.controllers[i]) {
                        batch.controllers[i]->Update(batch.deltaTime);
                    }
                }
                promise->set_value();
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };
        
        return SubmitTask(std::move(batchTask), batch.priority);
    }

    void AnimationThreadPool::SubmitBatches(const std::vector<AnimationBatch>& batches) {
        for (const auto& batch : batches) {
            SubmitBatch(batch);
        }
    }

    void AnimationThreadPool::WaitForAll() {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_completionCondition.wait(lock, [this] {
            return m_taskQueue.empty() && m_stats.activeThreads == 0;
        });
    }

    void AnimationThreadPool::WaitForCompletion() {
        WaitForAll();
    }

    bool AnimationThreadPool::IsIdle() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
        return m_taskQueue.empty() && m_stats.activeThreads == 0;
    }

    void AnimationThreadPool::SetMaxThreads(size_t maxThreads) {
        if (maxThreads != m_config.numThreads) {
            LOG_INFO("Changing thread pool size requires restart");
            // Would need to implement dynamic thread pool resizing
        }
    }

    void AnimationThreadPool::SetQueueSize(size_t maxQueueSize) {
        m_config.maxQueueSize = maxQueueSize;
    }

    size_t AnimationThreadPool::GetQueueSize() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
        return m_taskQueue.size();
    }

    AnimationThreadPool::ThreadPoolStats AnimationThreadPool::GetStats() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_statsMutex));
        ThreadPoolStats stats = m_stats;
        
        // Calculate current queue size
        {
            std::lock_guard<std::mutex> queueLock(const_cast<std::mutex&>(m_queueMutex));
            stats.currentQueueSize = m_taskQueue.size();
        }
        
        return stats;
    }

    void AnimationThreadPool::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = ThreadPoolStats{};
        m_lastStatsUpdate = std::chrono::steady_clock::now();
    }

    void AnimationThreadPool::PauseThreads() {
        m_paused = true;
        LOG_DEBUG("Animation thread pool paused");
    }

    void AnimationThreadPool::ResumeThreads() {
        m_paused = false;
        m_queueCondition.notify_all();
        LOG_DEBUG("Animation thread pool resumed");
    }

    // Private methods
    void AnimationThreadPool::WorkerThread(size_t threadId) {
        LOG_DEBUG("Animation worker thread started");
        
        while (!m_shutdown) {
            AnimationTask task;
            
            // Wait for a task or shutdown signal
            if (!GetNextTask(task, threadId)) {
                continue;
            }
            
            // Process the task
            {
                std::lock_guard<std::mutex> lock(m_statsMutex);
                m_stats.activeThreads++;
            }
            
            auto startTime = std::chrono::high_resolution_clock::now();
            ProcessTask(task);
            auto endTime = std::chrono::high_resolution_clock::now();
            
            float processingTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
            UpdateStats(task, processingTime);
            
            {
                std::lock_guard<std::mutex> lock(m_statsMutex);
                m_stats.activeThreads--;
            }
            
            // Notify completion condition
            m_completionCondition.notify_all();
        }
        
        LOG_DEBUG("Animation worker thread stopped");
    }

    bool AnimationThreadPool::GetNextTask(AnimationTask& task, size_t threadId) {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        
        // Wait for a task or shutdown
        m_queueCondition.wait(lock, [this] {
            return !m_taskQueue.empty() || m_shutdown || m_paused;
        });
        
        if (m_shutdown) {
            return false;
        }
        
        if (m_paused) {
            return false;
        }
        
        if (!m_taskQueue.empty()) {
            task = m_taskQueue.top();
            m_taskQueue.pop();
            return true;
        }
        
        // Try work stealing if enabled
        if (m_config.enableWorkStealing) {
            lock.unlock();
            return StealWork(task, threadId);
        }
        
        return false;
    }

    void AnimationThreadPool::ProcessTask(const AnimationTask& task) {
        try {
            task.task();
        } catch (const std::exception& e) {
            LOG_ERROR("Animation task failed with exception");
        } catch (...) {
            LOG_ERROR("Animation task failed with unknown exception");
        }
    }

    void AnimationThreadPool::UpdateStats(const AnimationTask& task, float processingTime) {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        
        m_stats.totalTasksProcessed++;
        
        // Update average task time
        m_stats.averageTaskTime = (m_stats.averageTaskTime * (m_stats.totalTasksProcessed - 1) + processingTime) / m_stats.totalTasksProcessed;
        
        // Calculate queue time
        auto currentTime = std::chrono::steady_clock::now();
        float queueTime = std::chrono::duration<float, std::milli>(currentTime - task.submitTime).count();
        m_stats.averageQueueTime = (m_stats.averageQueueTime * (m_stats.totalTasksProcessed - 1) + queueTime) / m_stats.totalTasksProcessed;
        
        // Update tasks per second
        auto timeSinceLastUpdate = std::chrono::duration<float>(currentTime - m_lastStatsUpdate).count();
        if (timeSinceLastUpdate >= 1.0f) {
            m_stats.tasksPerSecond = static_cast<size_t>(m_stats.totalTasksProcessed / timeSinceLastUpdate);
            m_lastStatsUpdate = currentTime;
        }
    }

    bool AnimationThreadPool::StealWork(AnimationTask& task, size_t threadId) {
        // Try to steal work from other threads
        for (size_t i = 0; i < m_workStealingQueues.size(); ++i) {
            if (i == threadId) continue;
            
            std::lock_guard<std::mutex> lock(*m_workStealingMutexes[i]);
            if (!m_workStealingQueues[i].empty()) {
                task = m_workStealingQueues[i].front();
                m_workStealingQueues[i].pop();
                return true;
            }
        }
        
        return false;
    }

    void AnimationThreadPool::DistributeWork() {
        // Distribute work among work stealing queues
        // This would be called periodically to balance load
    }

    // MultiThreadedAnimationManager implementation
    MultiThreadedAnimationManager::MultiThreadedAnimationManager() {
        LOG_INFO("MultiThreadedAnimationManager created");
    }

    MultiThreadedAnimationManager::~MultiThreadedAnimationManager() {
        Shutdown();
        LOG_INFO("MultiThreadedAnimationManager destroyed");
    }

    bool MultiThreadedAnimationManager::Initialize(const AnimationThreadConfig& config) {
        LOG_INFO("Initializing MultiThreadedAnimationManager");
        
        // Initialize thread pool
        m_threadPool = std::make_unique<AnimationThreadPool>();
        if (!m_threadPool->Initialize(config)) {
            LOG_ERROR("Failed to initialize animation thread pool");
            return false;
        }
        
        m_lastUpdateTime = std::chrono::steady_clock::now();
        
        LOG_INFO("MultiThreadedAnimationManager initialized successfully");
        return true;
    }

    void MultiThreadedAnimationManager::Shutdown() {
        LOG_INFO("Shutting down MultiThreadedAnimationManager");
        
        if (m_threadPool) {
            m_threadPool->Shutdown();
            m_threadPool.reset();
        }
        
        m_instances.clear();
        
        LOG_INFO("MultiThreadedAnimationManager shutdown complete");
    }

    uint32_t MultiThreadedAnimationManager::RegisterAnimationController(std::shared_ptr<AnimationController> controller,
                                                                       AnimationTaskPriority priority) {
        if (!controller) {
            LOG_ERROR("Cannot register null animation controller");
            return 0;
        }
        
        std::lock_guard<std::mutex> lock(m_instancesMutex);
        
        uint32_t instanceId = m_nextInstanceId++;
        
        AnimationInstance instance;
        instance.controller = controller;
        instance.priority = priority;
        instance.instanceId = instanceId;
        instance.needsUpdate = true;
        instance.lastUpdateTime = 0.0f;
        
        m_instances[instanceId] = instance;
        
        LOG_DEBUG("Registered animation controller for multi-threading");
        return instanceId;
    }

    void MultiThreadedAnimationManager::UnregisterAnimationController(uint32_t instanceId) {
        std::lock_guard<std::mutex> lock(m_instancesMutex);
        
        auto it = m_instances.find(instanceId);
        if (it != m_instances.end()) {
            m_instances.erase(it);
            LOG_DEBUG("Unregistered animation controller from multi-threading");
        } else {
            LOG_WARNING("Attempted to unregister non-existent animation controller");
        }
    }

    void MultiThreadedAnimationManager::SetInstancePriority(uint32_t instanceId, AnimationTaskPriority priority) {
        std::lock_guard<std::mutex> lock(m_instancesMutex);
        
        auto it = m_instances.find(instanceId);
        if (it != m_instances.end()) {
            it->second.priority = priority;
        }
    }

    void MultiThreadedAnimationManager::UpdateAnimations(float deltaTime) {
        if (!m_threadingEnabled || !m_threadPool) {
            // Fall back to single-threaded update
            std::lock_guard<std::mutex> lock(m_instancesMutex);
            for (auto& pair : m_instances) {
                if (pair.second.controller) {
                    pair.second.controller->Update(deltaTime);
                }
            }
            return;
        }
        
        UpdateAnimationsParallel(deltaTime);
    }

    void MultiThreadedAnimationManager::UpdateAnimationsParallel(float deltaTime) {
        auto startTime = std::chrono::steady_clock::now();
        
        // Collect instances that need updating
        std::vector<AnimationInstance*> instancesToUpdate;
        {
            std::lock_guard<std::mutex> lock(m_instancesMutex);
            instancesToUpdate.reserve(m_instances.size());
            
            for (auto& pair : m_instances) {
                AnimationInstance& instance = pair.second;
                
                // Check if instance needs update (could be based on LOD, distance, etc.)
                bool shouldUpdate = true;
                if (m_lodSystem) {
                    shouldUpdate = !m_lodSystem->IsInstanceCulled(instance.instanceId);
                }
                
                if (shouldUpdate && instance.needsUpdate) {
                    instancesToUpdate.push_back(&instance);
                }
            }
        }
        
        if (instancesToUpdate.empty()) {
            return;
        }
        
        // Sort by priority
        SortInstancesByPriority(instancesToUpdate);
        
        // Create batches
        std::vector<AnimationBatch> batches;
        CreateBatches(instancesToUpdate, deltaTime, batches);
        
        // Balance workload
        BalanceWorkload(batches);
        
        // Submit batches to thread pool
        std::vector<std::future<void>> futures;
        futures.reserve(batches.size());
        
        for (const auto& batch : batches) {
            futures.push_back(m_threadPool->SubmitBatch(batch));
        }
        
        // Wait for all batches to complete
        for (auto& future : futures) {
            future.wait();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        float updateTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.totalInstances = m_instances.size();
            m_stats.activeInstances = instancesToUpdate.size();
            m_stats.batchesProcessed += batches.size();
            m_stats.totalUpdateTime += updateTime;
            m_stats.averageUpdateTime = m_stats.totalUpdateTime / (m_stats.batchesProcessed > 0 ? m_stats.batchesProcessed : 1);
            
            // Calculate parallel efficiency (simplified)
            float sequentialTime = instancesToUpdate.size() * 1.0f; // Assume 1ms per instance
            m_stats.parallelEfficiency = sequentialTime / updateTime;
            
            if (m_threadPool) {
                m_stats.threadPoolStats = m_threadPool->GetStats();
            }
        }
    }

    void MultiThreadedAnimationManager::UpdateAnimationsBatched(float deltaTime, size_t batchSize) {
        if (batchSize == 0) {
            batchSize = m_maxBatchSize;
        }
        
        std::vector<AnimationInstance*> instancesToUpdate;
        {
            std::lock_guard<std::mutex> lock(m_instancesMutex);
            for (auto& pair : m_instances) {
                if (pair.second.needsUpdate) {
                    instancesToUpdate.push_back(&pair.second);
                }
            }
        }
        
        // Process in batches
        for (size_t i = 0; i < instancesToUpdate.size(); i += batchSize) {
            size_t endIdx = std::min(i + batchSize, instancesToUpdate.size());
            
            AnimationBatch batch;
            batch.deltaTime = deltaTime;
            batch.priority = AnimationTaskPriority::Normal;
            
            for (size_t j = i; j < endIdx; ++j) {
                batch.controllers.push_back(instancesToUpdate[j]->controller);
                batch.instanceIds.push_back(instancesToUpdate[j]->instanceId);
            }
            
            if (m_threadPool) {
                m_threadPool->SubmitBatch(batch);
            } else {
                ProcessBatch(batch);
            }
        }
    }

    void MultiThreadedAnimationManager::WaitForAnimationUpdates() {
        if (m_threadPool) {
            m_threadPool->WaitForAll();
        }
    }

    void MultiThreadedAnimationManager::FlushPendingUpdates() {
        WaitForAnimationUpdates();
    }

    MultiThreadedAnimationManager::AnimationManagerStats MultiThreadedAnimationManager::GetStats() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_statsMutex));
        return m_stats;
    }

    void MultiThreadedAnimationManager::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = AnimationManagerStats{};
        if (m_threadPool) {
            m_threadPool->ResetStats();
        }
    }

    void MultiThreadedAnimationManager::SetBatchSize(size_t minBatch, size_t maxBatch) {
        m_minBatchSize = minBatch;
        m_maxBatchSize = maxBatch;
    }

    // Private helper methods
    void MultiThreadedAnimationManager::CreateBatches(const std::vector<AnimationInstance*>& instances,
                                                     float deltaTime,
                                                     std::vector<AnimationBatch>& batches) {
        if (instances.empty()) {
            return;
        }
        
        size_t batchSize = std::clamp(instances.size() / m_threadPool->GetThreadCount(),
                                     m_minBatchSize, m_maxBatchSize);
        
        for (size_t i = 0; i < instances.size(); i += batchSize) {
            size_t endIdx = std::min(i + batchSize, instances.size());
            
            AnimationBatch batch;
            batch.deltaTime = deltaTime;
            batch.priority = instances[i]->priority; // Use first instance's priority
            
            for (size_t j = i; j < endIdx; ++j) {
                batch.controllers.push_back(instances[j]->controller);
                batch.instanceIds.push_back(instances[j]->instanceId);
            }
            
            batches.push_back(std::move(batch));
        }
    }

    void MultiThreadedAnimationManager::ProcessBatch(const AnimationBatch& batch) {
        for (size_t i = 0; i < batch.controllers.size(); ++i) {
            if (batch.controllers[i]) {
                batch.controllers[i]->Update(batch.deltaTime);
            }
        }
    }

    void MultiThreadedAnimationManager::UpdateStatistics() {
        // Statistics are updated in UpdateAnimationsParallel
    }

    void MultiThreadedAnimationManager::SortInstancesByPriority(std::vector<AnimationInstance*>& instances) {
        std::sort(instances.begin(), instances.end(),
                 [](const AnimationInstance* a, const AnimationInstance* b) {
                     return a->priority > b->priority;
                 });
    }

    void MultiThreadedAnimationManager::BalanceWorkload(std::vector<AnimationBatch>& batches) {
        // Simple load balancing - could be improved with more sophisticated algorithms
        if (batches.size() <= 1) {
            return;
        }
        
        // Sort batches by size to distribute work more evenly
        std::sort(batches.begin(), batches.end(),
                 [](const AnimationBatch& a, const AnimationBatch& b) {
                     return a.size() > b.size();
                 });
    }

    // GPUAnimationProcessor implementation (stub for now)
    GPUAnimationProcessor::GPUAnimationProcessor() {
        LOG_INFO("GPUAnimationProcessor created");
    }

    GPUAnimationProcessor::~GPUAnimationProcessor() {
        Shutdown();
        LOG_INFO("GPUAnimationProcessor destroyed");
    }

    bool GPUAnimationProcessor::Initialize() {
        LOG_INFO("Initializing GPUAnimationProcessor");
        
        // Check for compute shader support
        // This would require OpenGL context and extension checking
        m_supportsCompute = false; // Placeholder
        
        if (!m_supportsCompute) {
            LOG_WARNING("Compute shaders not supported, GPU acceleration disabled");
            return false;
        }
        
        // Create compute shader and buffers
        if (!CreateComputeShader() || !CreateBuffers()) {
            LOG_ERROR("Failed to create GPU resources");
            return false;
        }
        
        m_isInitialized = true;
        LOG_INFO("GPUAnimationProcessor initialized successfully");
        return true;
    }

    void GPUAnimationProcessor::Shutdown() {
        LOG_INFO("Shutting down GPUAnimationProcessor");
        
        CleanupGPUResources();
        m_gpuAnimationData.clear();
        m_isInitialized = false;
        
        LOG_INFO("GPUAnimationProcessor shutdown complete");
    }

    bool GPUAnimationProcessor::IsGPUAccelerationSupported() const {
        return m_supportsCompute;
    }

    bool GPUAnimationProcessor::IsComputeShaderSupported() const {
        return m_supportsCompute;
    }

    size_t GPUAnimationProcessor::GetMaxComputeWorkGroups() const {
        return m_maxWorkGroups;
    }

    size_t GPUAnimationProcessor::GetMaxComputeInvocations() const {
        return m_maxInvocations;
    }

    uint32_t GPUAnimationProcessor::UploadAnimationData(const GPUAnimationData& data) {
        if (!m_isInitialized) {
            return 0;
        }
        
        uint32_t dataId = m_nextDataId++;
        m_gpuAnimationData[dataId] = data;
        
        // Upload to GPU buffers would happen here
        UpdateGPUBuffers(data);
        
        return dataId;
    }

    void GPUAnimationProcessor::UpdateAnimationData(uint32_t dataId, const GPUAnimationData& data) {
        auto it = m_gpuAnimationData.find(dataId);
        if (it != m_gpuAnimationData.end()) {
            it->second = data;
            UpdateGPUBuffers(data);
        }
    }

    void GPUAnimationProcessor::RemoveAnimationData(uint32_t dataId) {
        m_gpuAnimationData.erase(dataId);
    }

    void GPUAnimationProcessor::ProcessAnimationsGPU(const std::vector<uint32_t>& dataIds, float deltaTime) {
        if (!m_isInitialized || dataIds.empty()) {
            return;
        }
        
        // Dispatch compute shader for each animation data set
        for (uint32_t dataId : dataIds) {
            auto it = m_gpuAnimationData.find(dataId);
            if (it != m_gpuAnimationData.end()) {
                DispatchComputeShader(it->second.animationCount, it->second.boneCount);
            }
        }
        
        // Read back results
        ReadbackResults();
    }

    void GPUAnimationProcessor::ProcessSkinnedMeshes(const std::vector<uint32_t>& meshIds, const std::vector<uint32_t>& animationIds) {
        // GPU skinning implementation would go here
        LOG_DEBUG("GPU skinning not yet implemented");
    }

    void GPUAnimationProcessor::WaitForGPUCompletion() {
        // GPU synchronization would go here
    }

    bool GPUAnimationProcessor::IsGPUProcessingComplete() const {
        // Check GPU processing status
        return true; // Placeholder
    }

    GPUAnimationProcessor::GPUProcessingStats GPUAnimationProcessor::GetStats() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_statsMutex));
        return m_stats;
    }

    void GPUAnimationProcessor::ResetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = GPUProcessingStats{};
    }

    void GPUAnimationProcessor::FlushGPUMemory() {
        // Flush GPU memory
    }

    size_t GPUAnimationProcessor::GetGPUMemoryUsage() const {
        return static_cast<size_t>(m_stats.totalGPUMemoryUsed);
    }

    void GPUAnimationProcessor::SetGPUMemoryBudget(size_t budgetBytes) {
        m_gpuMemoryBudget = budgetBytes;
    }

    // Private GPU methods (stubs)
    bool GPUAnimationProcessor::CreateComputeShader() {
        // Create compute shader for animation processing
        return false; // Not implemented yet
    }

    bool GPUAnimationProcessor::CreateBuffers() {
        // Create GPU buffers for animation data
        return false; // Not implemented yet
    }

    void GPUAnimationProcessor::CleanupGPUResources() {
        // Cleanup GPU resources
    }

    void GPUAnimationProcessor::UpdateGPUBuffers(const GPUAnimationData& data) {
        // Update GPU buffers with animation data
    }

    void GPUAnimationProcessor::DispatchComputeShader(size_t numAnimations, size_t numBones) {
        // Dispatch compute shader
    }

    void GPUAnimationProcessor::ReadbackResults() {
        // Read back results from GPU
    }

    // AnimationMemoryPool implementation
    AnimationMemoryPool::AnimationMemoryPool() {
        LOG_DEBUG("AnimationMemoryPool created");
    }

    AnimationMemoryPool::~AnimationMemoryPool() {
        Reset();
        LOG_DEBUG("AnimationMemoryPool destroyed");
    }

    void* AnimationMemoryPool::Allocate(size_t size, size_t alignment) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        MemoryBlock* block = FindFreeBlock(size, alignment);
        if (!block) {
            CreateNewBlock(size);
            block = FindFreeBlock(size, alignment);
        }
        
        if (block) {
            block->inUse = true;
            m_stats.totalAllocations++;
            m_stats.currentAllocations++;
            m_stats.totalBytesAllocated += size;
            m_stats.currentBytesAllocated += size;
            
            m_stats.peakAllocations = std::max(m_stats.peakAllocations, m_stats.currentAllocations);
            m_stats.peakBytesAllocated = std::max(m_stats.peakBytesAllocated, m_stats.currentBytesAllocated);
            
            return block->ptr;
        }
        
        return nullptr;
    }

    void AnimationMemoryPool::Deallocate(void* ptr) {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& block : m_blocks) {
            if (block.ptr == ptr && block.inUse) {
                block.inUse = false;
                m_stats.totalDeallocations++;
                m_stats.currentAllocations--;
                m_stats.currentBytesAllocated -= block.size;
                return;
            }
        }
    }

    void AnimationMemoryPool::Reset() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& block : m_blocks) {
            if (block.ptr) {
                _aligned_free(block.ptr);
            }
        }
        
        m_blocks.clear();
        m_stats.currentAllocations = 0;
        m_stats.currentBytesAllocated = 0;
    }

    void AnimationMemoryPool::Compact() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // Remove unused blocks
        m_blocks.erase(
            std::remove_if(m_blocks.begin(), m_blocks.end(),
                          [](const MemoryBlock& block) {
                              if (!block.inUse && block.ptr) {
                                  _aligned_free(block.ptr);
                                  return true;
                              }
                              return false;
                          }),
            m_blocks.end()
        );
    }

    size_t AnimationMemoryPool::GetTotalAllocated() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        return m_stats.currentBytesAllocated;
    }

    size_t AnimationMemoryPool::GetTotalCapacity() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        size_t totalCapacity = 0;
        for (const auto& block : m_blocks) {
            totalCapacity += block.size;
        }
        return totalCapacity;
    }

    AnimationMemoryPool::PoolStats AnimationMemoryPool::GetStats() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_mutex));
        return m_stats;
    }

    void AnimationMemoryPool::ResetStats() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stats = PoolStats{};
    }

    AnimationMemoryPool::MemoryBlock* AnimationMemoryPool::FindFreeBlock(size_t size, size_t alignment) {
        for (auto& block : m_blocks) {
            if (!block.inUse && block.size >= size && block.alignment >= alignment) {
                return &block;
            }
        }
        return nullptr;
    }

    void AnimationMemoryPool::CreateNewBlock(size_t size) {
        size_t blockSize = std::max(size, DEFAULT_BLOCK_SIZE);
        void* ptr = _aligned_malloc(blockSize, 16);
        
        if (ptr) {
            MemoryBlock block;
            block.ptr = ptr;
            block.size = blockSize;
            block.alignment = 16;
            block.inUse = false;
            
            m_blocks.push_back(block);
        }
    }

} // namespace Animation
} // namespace GameEngine