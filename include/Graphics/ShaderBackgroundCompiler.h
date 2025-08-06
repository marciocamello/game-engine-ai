#pragma once

#include "Graphics/ShaderVariant.h"
#include <string>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <unordered_map>
#include <future>
#include <chrono>

namespace GameEngine {
    class Shader;

    struct ShaderCompilationJob {
        std::string name;
        std::string vertexSource;
        std::string fragmentSource;
        std::string geometrySource;
        std::string computeSource;
        ShaderVariant variant;
        std::promise<std::shared_ptr<Shader>> promise;
        std::function<void(std::shared_ptr<Shader>)> callback;
        int priority = 0; // Higher values = higher priority
        
        ShaderCompilationJob() = default;
        ShaderCompilationJob(const ShaderCompilationJob&) = delete;
        ShaderCompilationJob& operator=(const ShaderCompilationJob&) = delete;
        ShaderCompilationJob(ShaderCompilationJob&&) = default;
        ShaderCompilationJob& operator=(ShaderCompilationJob&&) = default;
    };

    struct JobComparator {
        bool operator()(const std::unique_ptr<ShaderCompilationJob>& a, 
                       const std::unique_ptr<ShaderCompilationJob>& b) const {
            return a->priority < b->priority; // Higher priority first
        }
    };

    class ShaderBackgroundCompiler {
    public:
        // Singleton access
        static ShaderBackgroundCompiler& GetInstance();

        // Lifecycle
        bool Initialize();
        void Shutdown();

        // Job management
        std::future<std::shared_ptr<Shader>> SubmitCompilationJob(
            const std::string& name,
            const std::string& vertexSource,
            const std::string& fragmentSource,
            const std::string& geometrySource = "",
            const std::string& computeSource = "",
            const ShaderVariant& variant = ShaderVariant{},
            int priority = 0,
            std::function<void(std::shared_ptr<Shader>)> callback = nullptr
        );

        std::future<std::shared_ptr<Shader>> SubmitVariantCompilationJob(
            const std::string& baseName,
            const ShaderVariant& variant,
            int priority = 0,
            std::function<void(std::shared_ptr<Shader>)> callback = nullptr
        );

        // Progressive loading
        void StartProgressiveLoading(const std::vector<std::string>& shaderPaths);
        void StopProgressiveLoading();
        bool IsProgressiveLoadingActive() const { return m_progressiveLoadingActive; }

        // Background variant compilation
        void PrecompileVariants(const std::string& baseName, const std::vector<ShaderVariant>& variants);
        void PrecompileCommonVariants(); // Precompile commonly used variants

        // Job control
        void SetMaxWorkerThreads(size_t count);
        size_t GetMaxWorkerThreads() const { return m_maxWorkerThreads; }
        void PauseCompilation();
        void ResumeCompilation();
        bool IsCompilationPaused() const { return m_paused; }

        // Statistics and monitoring
        struct CompilationStats {
            size_t totalJobsSubmitted = 0;
            size_t totalJobsCompleted = 0;
            size_t totalJobsFailed = 0;
            size_t currentQueueSize = 0;
            size_t activeWorkers = 0;
            double averageCompilationTime = 0.0;
            double totalCompilationTime = 0.0;
        };

        CompilationStats GetStats() const;
        void ResetStats();

        // Priority management
        void SetJobPriority(const std::string& name, int priority);
        void CancelJob(const std::string& name);
        void CancelAllJobs();

        // Memory management
        void SetMaxCacheSize(size_t maxSizeBytes);
        void ClearCompilationCache();
        size_t GetCacheSize() const;

    private:
        ShaderBackgroundCompiler() = default;
        ~ShaderBackgroundCompiler() = default;
        ShaderBackgroundCompiler(const ShaderBackgroundCompiler&) = delete;
        ShaderBackgroundCompiler& operator=(const ShaderBackgroundCompiler&) = delete;

        // Worker thread management
        void WorkerThreadFunction();
        void StartWorkerThreads();
        void StopWorkerThreads();

        // Job processing
        std::shared_ptr<Shader> CompileShaderJob(const ShaderCompilationJob& job);
        void ProcessCompletedJob(std::unique_ptr<ShaderCompilationJob> job, std::shared_ptr<Shader> shader);

        // Progressive loading
        void ProgressiveLoadingThreadFunction();
        void LoadNextShaderBatch();

        // Variant precompilation
        void PrecompileVariantBatch(const std::string& baseName, const std::vector<ShaderVariant>& variants);

        // Cache management
        void UpdateCacheSize();
        void EvictOldestCacheEntries();

        // Member variables
        std::atomic<bool> m_initialized{false};
        std::atomic<bool> m_shutdown{false};
        std::atomic<bool> m_paused{false};

        // Worker threads
        std::vector<std::thread> m_workerThreads;
        size_t m_maxWorkerThreads = 2; // Default to 2 background threads

        // Job queue
        std::priority_queue<std::unique_ptr<ShaderCompilationJob>, 
                           std::vector<std::unique_ptr<ShaderCompilationJob>>, 
                           JobComparator> m_jobQueue;
        mutable std::mutex m_queueMutex;
        std::condition_variable m_queueCondition;

        // Job tracking
        std::unordered_map<std::string, std::unique_ptr<ShaderCompilationJob>> m_activeJobs;
        std::mutex m_activeJobsMutex;

        // Progressive loading
        std::atomic<bool> m_progressiveLoadingActive{false};
        std::thread m_progressiveLoadingThread;
        std::vector<std::string> m_progressiveLoadingQueue;
        std::mutex m_progressiveLoadingMutex;
        size_t m_progressiveLoadingIndex = 0;
        size_t m_progressiveLoadingBatchSize = 4; // Load 4 shaders per batch

        // Compilation cache
        std::unordered_map<std::string, std::shared_ptr<Shader>> m_compilationCache;
        mutable std::mutex m_cacheMutex;
        size_t m_maxCacheSize = 100 * 1024 * 1024; // 100MB default
        size_t m_currentCacheSize = 0;

        // Statistics
        mutable std::mutex m_statsMutex;
        CompilationStats m_stats;
        std::chrono::high_resolution_clock::time_point m_lastStatsUpdate;

        // Shader source storage for variants
        struct ShaderSources {
            std::string vertexSource;
            std::string fragmentSource;
            std::string geometrySource;
            std::string computeSource;
        };
        std::unordered_map<std::string, ShaderSources> m_shaderSources;
        std::mutex m_sourcesMutex;
    };
}