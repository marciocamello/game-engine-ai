#pragma once

#include "Resource/ModelLoader.h"
#include "Graphics/Model.h"
#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <memory>
#include <type_traits>

namespace GameEngine {

    /**
     * @brief Thread pool for managing worker threads
     */
    class ThreadPool {
    public:
        ThreadPool(size_t numThreads = std::thread::hardware_concurrency());
        ~ThreadPool();

        // Add task to the queue
        template<typename F, typename... Args>
        auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

        // Get number of active threads
        size_t GetThreadCount() const { return m_workers.size(); }

        // Stop all threads
        void Stop();

    private:
        std::vector<std::thread> m_workers;
        std::queue<std::function<void()>> m_tasks;
        std::mutex m_queueMutex;
        std::condition_variable m_condition;
        std::atomic<bool> m_stop{false};
    };

    /**
     * @brief Asynchronous model loading system with progress tracking and cancellation
     */
    class AsyncModelLoader {
    public:
        /**
         * @brief Progress callback function type
         * Parameters: filepath, progress (0.0-1.0), stage description
         */
        using ProgressCallback = std::function<void(const std::string&, float, const std::string&)>;

        /**
         * @brief Task priority levels
         */
        enum class TaskPriority {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        };

        /**
         * @brief Load task information
         */
        struct LoadTask {
            std::string filepath;
            ModelLoader::LoadingFlags flags;
            std::promise<std::shared_ptr<Model>> promise;
            std::atomic<float> progress{0.0f};
            std::atomic<bool> cancelled{false};
            std::string currentStage;
            std::chrono::steady_clock::time_point startTime;
            TaskPriority priority = TaskPriority::Normal;
            std::vector<std::string> dependencies; // Files that must be loaded first
            size_t estimatedMemoryUsage = 0; // Estimated memory usage in bytes
            
            LoadTask(const std::string& path, ModelLoader::LoadingFlags loadFlags, TaskPriority prio = TaskPriority::Normal)
                : filepath(path), flags(loadFlags), startTime(std::chrono::steady_clock::now()), priority(prio) {}
        };

        /**
         * @brief Loading statistics
         */
        struct LoadingStats {
            uint32_t totalLoadsStarted = 0;
            uint32_t totalLoadsCompleted = 0;
            uint32_t totalLoadsCancelled = 0;
            uint32_t totalLoadsFailed = 0;
            uint32_t currentActiveLoads = 0;
            uint32_t queuedLoads = 0;
            float averageLoadTimeMs = 0.0f;
            size_t totalMemoryLoaded = 0;
            size_t currentMemoryUsage = 0;
            size_t peakMemoryUsage = 0;
        };

    public:
        // Lifecycle
        AsyncModelLoader();
        ~AsyncModelLoader();

        bool Initialize(uint32_t workerThreadCount = 0);
        void Shutdown();
        bool IsInitialized() const { return m_initialized; }

        // Asynchronous loading
        std::future<std::shared_ptr<Model>> LoadModelAsync(const std::string& filepath);
        std::future<std::shared_ptr<Model>> LoadModelAsync(const std::string& filepath, ModelLoader::LoadingFlags flags);
        std::future<std::shared_ptr<Model>> LoadModelAsync(const std::string& filepath, ModelLoader::LoadingFlags flags, TaskPriority priority);
        std::future<std::shared_ptr<Model>> LoadModelAsync(const std::string& filepath, ModelLoader::LoadingFlags flags, TaskPriority priority, const std::vector<std::string>& dependencies);
        std::future<std::vector<std::shared_ptr<Model>>> LoadModelsAsync(const std::vector<std::string>& filepaths);
        std::future<std::vector<std::shared_ptr<Model>>> LoadModelsAsync(const std::vector<std::string>& filepaths, ModelLoader::LoadingFlags flags);
        std::future<std::vector<std::shared_ptr<Model>>> LoadModelsAsync(const std::vector<std::string>& filepaths, ModelLoader::LoadingFlags flags, TaskPriority priority);

        // Progress tracking
        void SetProgressCallback(ProgressCallback callback);
        float GetLoadingProgress(const std::string& filepath) const;
        std::string GetLoadingStage(const std::string& filepath) const;
        std::vector<std::string> GetActiveLoads() const;
        bool IsLoading(const std::string& filepath) const;

        // Load management
        bool CancelLoad(const std::string& filepath);
        void CancelAllLoads();
        void WaitForAllLoads();
        void SetMaxConcurrentLoads(uint32_t maxLoads);
        uint32_t GetMaxConcurrentLoads() const { return m_maxConcurrentLoads; }

        // Thread management
        void SetWorkerThreadCount(uint32_t count);
        uint32_t GetWorkerThreadCount() const;

        // Configuration
        void SetDefaultLoadingFlags(ModelLoader::LoadingFlags flags);
        ModelLoader::LoadingFlags GetDefaultLoadingFlags() const { return m_defaultFlags; }
        void SetMemoryLimit(size_t maxMemoryBytes);
        size_t GetMemoryLimit() const { return m_maxMemoryUsage; }
        void SetDefaultPriority(TaskPriority priority);
        TaskPriority GetDefaultPriority() const { return m_defaultPriority; }

        // Statistics and debugging
        LoadingStats GetLoadingStats() const;
        void ResetStats();
        void SetVerboseLogging(bool enabled) { m_verboseLogging = enabled; }

        // Resource cleanup
        void CleanupCompletedTasks();
        void FreeMemoryIfNeeded();
        
        // Queue management
        std::vector<std::string> GetQueuedTasks() const;
        bool HasDependenciesResolved(const std::string& filepath) const;
        void ProcessTaskQueue();

    private:
        std::unique_ptr<ThreadPool> m_threadPool;
        std::unique_ptr<ModelLoader> m_modelLoader;
        
        // Task management
        std::unordered_map<std::string, std::shared_ptr<LoadTask>> m_activeTasks;
        std::queue<std::shared_ptr<LoadTask>> m_taskQueue; // Priority queue for pending tasks
        std::unordered_map<std::string, std::shared_ptr<Model>> m_loadedModels; // Cache of loaded models
        mutable std::mutex m_tasksMutex;
        mutable std::mutex m_queueMutex;
        mutable std::mutex m_cacheMutex;
        
        // Configuration
        uint32_t m_maxConcurrentLoads = 4;
        size_t m_maxMemoryUsage = 1024 * 1024 * 1024; // 1GB default
        ModelLoader::LoadingFlags m_defaultFlags = ModelLoader::LoadingFlags::None;
        TaskPriority m_defaultPriority = TaskPriority::Normal;
        bool m_verboseLogging = false;
        bool m_initialized = false;

        // Progress tracking
        ProgressCallback m_progressCallback;
        mutable std::mutex m_progressMutex;

        // Statistics and memory tracking
        mutable LoadingStats m_stats;
        mutable std::mutex m_statsMutex;
        std::vector<float> m_loadTimes; // For calculating average
        std::atomic<size_t> m_currentMemoryUsage{0};
        std::atomic<size_t> m_peakMemoryUsage{0};

        // Helper methods
        void ProcessLoadTask(std::shared_ptr<LoadTask> task);
        void UpdateProgress(const std::string& filepath, float progress, const std::string& stage);
        void UpdateStats(bool completed, bool cancelled, bool failed, float loadTimeMs, size_t memoryUsed);
        void LogTaskStart(const std::string& filepath);
        void LogTaskComplete(const std::string& filepath, bool success, float timeMs);
        bool CanStartNewLoad() const;
        void WaitForLoadSlot();
        std::shared_ptr<Model> LoadModelInternal(const std::string& filepath, ModelLoader::LoadingFlags flags, std::shared_ptr<LoadTask> task);
        
        // Queue and dependency management
        void QueueTask(std::shared_ptr<LoadTask> task);
        std::shared_ptr<LoadTask> GetNextQueuedTask();
        bool AreDependenciesResolved(const std::shared_ptr<LoadTask>& task) const;
        void SortTaskQueue(); // Sort by priority
        
        // Memory management
        void UpdateMemoryUsage(size_t memoryDelta, bool increase);
        void EvictLeastRecentlyUsedModels();
        size_t EstimateModelMemoryUsage(const std::string& filepath) const;
    };

    // Template implementation for ThreadPool::Enqueue
    template<typename F, typename... Args>
    auto ThreadPool::Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            
            if (m_stop) {
                throw std::runtime_error("ThreadPool is stopped");
            }
            
            m_tasks.emplace([task]() { (*task)(); });
        }
        
        m_condition.notify_one();
        return result;
    }
}