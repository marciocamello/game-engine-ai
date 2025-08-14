#pragma once

#include "Animation/AnimationController.h"
#include "Animation/AnimationLOD.h"
#include "Core/Math.h"
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <queue>
#include <functional>
#include <chrono>

namespace GameEngine {
namespace Animation {

    /**
     * Thread pool configuration for animation processing
     */
    struct AnimationThreadConfig {
        size_t numThreads = 0;              // 0 = auto-detect based on hardware
        size_t maxQueueSize = 1000;         // Maximum number of queued tasks
        bool enableWorkStealing = true;     // Enable work stealing between threads
        bool enablePriority = true;         // Enable priority-based task scheduling
        size_t minBatchSize = 1;            // Minimum batch size for parallel processing
        size_t maxBatchSize = 32;           // Maximum batch size for parallel processing
        
        // Performance tuning
        bool enableCPUAffinity = false;     // Bind threads to specific CPU cores
        int baseCPUCore = 0;                // Base CPU core for thread affinity
        
        // Memory management
        size_t stackSize = 0;               // Thread stack size (0 = default)
        bool enableMemoryPooling = true;   // Enable memory pooling for tasks
    };

    /**
     * Animation task priority levels
     */
    enum class AnimationTaskPriority {
        Low = 0,
        Normal = 1,
        High = 2,
        Critical = 3
    };

    /**
     * Animation processing task
     */
    struct AnimationTask {
        std::function<void()> task;
        AnimationTaskPriority priority = AnimationTaskPriority::Normal;
        uint32_t instanceId = 0;
        float deltaTime = 0.0f;
        std::chrono::steady_clock::time_point submitTime;
        
        AnimationTask() = default;
        AnimationTask(std::function<void()> t, AnimationTaskPriority p = AnimationTaskPriority::Normal)
            : task(std::move(t)), priority(p), submitTime(std::chrono::steady_clock::now()) {}
        
        bool operator<(const AnimationTask& other) const {
            return priority < other.priority;
        }
    };

    /**
     * Animation batch processing data
     */
    struct AnimationBatch {
        std::vector<std::shared_ptr<AnimationController>> controllers;
        std::vector<uint32_t> instanceIds;
        float deltaTime = 0.0f;
        AnimationTaskPriority priority = AnimationTaskPriority::Normal;
        
        size_t size() const { return controllers.size(); }
        bool empty() const { return controllers.empty(); }
        void clear() { controllers.clear(); instanceIds.clear(); }
    };

    /**
     * Thread-safe animation thread pool for parallel animation processing
     */
    class AnimationThreadPool {
    public:
        // Lifecycle
        AnimationThreadPool();
        ~AnimationThreadPool();
        bool Initialize(const AnimationThreadConfig& config = AnimationThreadConfig{});
        void Shutdown();

        // Task submission
        std::future<void> SubmitTask(AnimationTask task);
        std::future<void> SubmitTask(std::function<void()> task, AnimationTaskPriority priority = AnimationTaskPriority::Normal);
        
        // Batch processing
        std::future<void> SubmitBatch(const AnimationBatch& batch);
        void SubmitBatches(const std::vector<AnimationBatch>& batches);

        // Synchronization
        void WaitForAll();
        void WaitForCompletion();
        bool IsIdle() const;

        // Configuration
        void SetMaxThreads(size_t maxThreads);
        void SetQueueSize(size_t maxQueueSize);
        size_t GetThreadCount() const { return m_threads.size(); }
        size_t GetQueueSize() const;

        // Statistics
        struct ThreadPoolStats {
            size_t totalTasksProcessed = 0;
            size_t totalTasksQueued = 0;
            size_t currentQueueSize = 0;
            size_t activeThreads = 0;
            float averageTaskTime = 0.0f;
            float averageQueueTime = 0.0f;
            size_t tasksPerSecond = 0;
        };
        
        ThreadPoolStats GetStats() const;
        void ResetStats();

        // Thread management
        void PauseThreads();
        void ResumeThreads();
        bool IsPaused() const { return m_paused; }

    private:
        // Configuration
        AnimationThreadConfig m_config;
        
        // Thread management
        std::vector<std::thread> m_threads;
        std::atomic<bool> m_shutdown{false};
        std::atomic<bool> m_paused{false};
        
        // Task queue
        std::priority_queue<AnimationTask> m_taskQueue;
        std::mutex m_queueMutex;
        std::condition_variable m_queueCondition;
        std::condition_variable m_completionCondition;
        
        // Statistics
        mutable std::mutex m_statsMutex;
        ThreadPoolStats m_stats;
        std::chrono::steady_clock::time_point m_lastStatsUpdate;
        
        // Work stealing queues (one per thread)
        std::vector<std::queue<AnimationTask>> m_workStealingQueues;
        std::vector<std::unique_ptr<std::mutex>> m_workStealingMutexes;
        
        // Thread worker function
        void WorkerThread(size_t threadId);
        
        // Task processing
        bool GetNextTask(AnimationTask& task, size_t threadId);
        void ProcessTask(const AnimationTask& task);
        void UpdateStats(const AnimationTask& task, float processingTime);
        
        // Work stealing
        bool StealWork(AnimationTask& task, size_t threadId);
        void DistributeWork();
    };

    /**
     * Multi-threaded animation manager for parallel animation processing
     */
    class MultiThreadedAnimationManager {
    public:
        // Lifecycle
        MultiThreadedAnimationManager();
        ~MultiThreadedAnimationManager();
        bool Initialize(const AnimationThreadConfig& config = AnimationThreadConfig{});
        void Shutdown();

        // Animation instance management
        uint32_t RegisterAnimationController(std::shared_ptr<AnimationController> controller,
                                           AnimationTaskPriority priority = AnimationTaskPriority::Normal);
        void UnregisterAnimationController(uint32_t instanceId);
        void SetInstancePriority(uint32_t instanceId, AnimationTaskPriority priority);

        // Batch processing
        void UpdateAnimations(float deltaTime);
        void UpdateAnimationsParallel(float deltaTime);
        void UpdateAnimationsBatched(float deltaTime, size_t batchSize = 0);

        // Synchronization
        void WaitForAnimationUpdates();
        void FlushPendingUpdates();

        // Performance monitoring
        struct AnimationManagerStats {
            size_t totalInstances = 0;
            size_t activeInstances = 0;
            size_t batchesProcessed = 0;
            float totalUpdateTime = 0.0f;
            float averageUpdateTime = 0.0f;
            float parallelEfficiency = 0.0f;  // Parallel speedup factor
            AnimationThreadPool::ThreadPoolStats threadPoolStats;
        };
        
        AnimationManagerStats GetStats() const;
        void ResetStats();

        // Configuration
        void SetBatchSize(size_t minBatch, size_t maxBatch);
        void SetThreadingEnabled(bool enabled) { m_threadingEnabled = enabled; }
        bool IsThreadingEnabled() const { return m_threadingEnabled; }

        // Integration with LOD system
        void SetLODSystem(std::shared_ptr<AnimationLODSystem> lodSystem) { m_lodSystem = lodSystem; }
        std::shared_ptr<AnimationLODSystem> GetLODSystem() const { return m_lodSystem; }

    private:
        // Core systems
        std::unique_ptr<AnimationThreadPool> m_threadPool;
        std::shared_ptr<AnimationLODSystem> m_lodSystem;
        
        // Instance management
        struct AnimationInstance {
            std::shared_ptr<AnimationController> controller;
            AnimationTaskPriority priority;
            uint32_t instanceId;
            bool needsUpdate = true;
            float lastUpdateTime = 0.0f;
        };
        
        std::unordered_map<uint32_t, AnimationInstance> m_instances;
        std::mutex m_instancesMutex;
        uint32_t m_nextInstanceId = 1;
        
        // Configuration
        bool m_threadingEnabled = true;
        size_t m_minBatchSize = 4;
        size_t m_maxBatchSize = 32;
        
        // Statistics
        mutable std::mutex m_statsMutex;
        AnimationManagerStats m_stats;
        std::chrono::steady_clock::time_point m_lastUpdateTime;
        
        // Helper methods
        void CreateBatches(const std::vector<AnimationInstance*>& instances, 
                          float deltaTime, 
                          std::vector<AnimationBatch>& batches);
        void ProcessBatch(const AnimationBatch& batch);
        void UpdateStatistics();
        
        // Priority-based sorting
        void SortInstancesByPriority(std::vector<AnimationInstance*>& instances);
        
        // Load balancing
        void BalanceWorkload(std::vector<AnimationBatch>& batches);
    };

    /**
     * GPU-accelerated animation processing (compute shader based)
     */
    class GPUAnimationProcessor {
    public:
        // Lifecycle
        GPUAnimationProcessor();
        ~GPUAnimationProcessor();
        bool Initialize();
        void Shutdown();

        // GPU processing capabilities
        bool IsGPUAccelerationSupported() const;
        bool IsComputeShaderSupported() const;
        size_t GetMaxComputeWorkGroups() const;
        size_t GetMaxComputeInvocations() const;

        // Animation data upload
        struct GPUAnimationData {
            std::vector<Math::Mat4> boneMatrices;
            std::vector<Math::Mat4> bindPoses;
            std::vector<Math::Mat4> inverseBindPoses;
            std::vector<float> animationWeights;
            uint32_t boneCount = 0;
            uint32_t animationCount = 0;
        };
        
        uint32_t UploadAnimationData(const GPUAnimationData& data);
        void UpdateAnimationData(uint32_t dataId, const GPUAnimationData& data);
        void RemoveAnimationData(uint32_t dataId);

        // GPU processing
        void ProcessAnimationsGPU(const std::vector<uint32_t>& dataIds, float deltaTime);
        void ProcessSkinnedMeshes(const std::vector<uint32_t>& meshIds, const std::vector<uint32_t>& animationIds);

        // Synchronization
        void WaitForGPUCompletion();
        bool IsGPUProcessingComplete() const;

        // Performance monitoring
        struct GPUProcessingStats {
            size_t totalAnimationsProcessed = 0;
            size_t totalVerticesProcessed = 0;
            float gpuProcessingTime = 0.0f;
            float cpuToGpuTransferTime = 0.0f;
            float gpuToCpuTransferTime = 0.0f;
            float totalGPUMemoryUsed = 0.0f;
            size_t computeShaderInvocations = 0;
        };
        
        GPUProcessingStats GetStats() const;
        void ResetStats();

        // Memory management
        void FlushGPUMemory();
        size_t GetGPUMemoryUsage() const;
        void SetGPUMemoryBudget(size_t budgetBytes);

    private:
        // GPU resources
        uint32_t m_computeProgram = 0;
        uint32_t m_boneMatricesBuffer = 0;
        uint32_t m_animationDataBuffer = 0;
        uint32_t m_resultBuffer = 0;
        
        // GPU capabilities
        bool m_isInitialized = false;
        bool m_supportsCompute = false;
        size_t m_maxWorkGroups = 0;
        size_t m_maxInvocations = 0;
        size_t m_gpuMemoryBudget = 256 * 1024 * 1024; // 256MB default
        
        // Data management
        std::unordered_map<uint32_t, GPUAnimationData> m_gpuAnimationData;
        uint32_t m_nextDataId = 1;
        
        // Statistics
        mutable std::mutex m_statsMutex;
        GPUProcessingStats m_stats;
        
        // Helper methods
        bool CreateComputeShader();
        bool CreateBuffers();
        void CleanupGPUResources();
        void UpdateGPUBuffers(const GPUAnimationData& data);
        
        // Compute shader management
        void DispatchComputeShader(size_t numAnimations, size_t numBones);
        void ReadbackResults();
    };

    /**
     * Memory pool for animation processing to reduce allocations
     */
    class AnimationMemoryPool {
    public:
        AnimationMemoryPool();
        ~AnimationMemoryPool();
        
        // Memory allocation
        void* Allocate(size_t size, size_t alignment = 16);
        void Deallocate(void* ptr);
        
        // Typed allocation
        template<typename T>
        T* Allocate(size_t count = 1) {
            return static_cast<T*>(Allocate(sizeof(T) * count, alignof(T)));
        }
        
        template<typename T>
        void Deallocate(T* ptr) {
            Deallocate(static_cast<void*>(ptr));
        }
        
        // Pool management
        void Reset();
        void Compact();
        size_t GetTotalAllocated() const;
        size_t GetTotalCapacity() const;
        
        // Statistics
        struct PoolStats {
            size_t totalAllocations = 0;
            size_t totalDeallocations = 0;
            size_t currentAllocations = 0;
            size_t peakAllocations = 0;
            size_t totalBytesAllocated = 0;
            size_t currentBytesAllocated = 0;
            size_t peakBytesAllocated = 0;
        };
        
        PoolStats GetStats() const;
        void ResetStats();
        
    private:
        struct MemoryBlock {
            void* ptr = nullptr;
            size_t size = 0;
            size_t alignment = 0;
            bool inUse = false;
        };
        
        std::vector<MemoryBlock> m_blocks;
        std::mutex m_mutex;
        PoolStats m_stats;
        
        static constexpr size_t DEFAULT_BLOCK_SIZE = 64 * 1024; // 64KB blocks
        
        MemoryBlock* FindFreeBlock(size_t size, size_t alignment);
        void CreateNewBlock(size_t size);
    };

} // namespace Animation
} // namespace GameEngine