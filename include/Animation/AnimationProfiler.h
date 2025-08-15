#pragma once

#include "Core/Math.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <chrono>
#include <functional>

namespace GameEngine {
    namespace Animation {
        class AnimationController;
        class AnimationSkeleton;
        class AnimationStateMachine;
        class BlendTree;
        class IKSolver;

        /**
         * @brief Performance timing data for animation operations
         */
        struct AnimationTimingData {
            std::string operationName;
            double averageTimeMs = 0.0;
            double minTimeMs = 0.0;
            double maxTimeMs = 0.0;
            double totalTimeMs = 0.0;
            uint32_t sampleCount = 0;
            double lastTimeMs = 0.0;
            
            void AddSample(double timeMs);
            void Reset();
        };

        /**
         * @brief Memory usage statistics for animation data
         */
        struct AnimationMemoryStats {
            size_t skeletonMemory = 0;
            size_t animationDataMemory = 0;
            size_t stateMachineMemory = 0;
            size_t blendTreeMemory = 0;
            size_t ikSolverMemory = 0;
            size_t morphTargetMemory = 0;
            size_t totalMemory = 0;
            
            void CalculateTotal();
        };

        /**
         * @brief Performance statistics for animation system
         */
        struct AnimationPerformanceStats {
            // Timing statistics
            AnimationTimingData skeletonUpdate;
            AnimationTimingData poseEvaluation;
            AnimationTimingData blending;
            AnimationTimingData ikSolving;
            AnimationTimingData morphTargetApplication;
            AnimationTimingData stateMachineUpdate;
            AnimationTimingData totalAnimationUpdate;
            
            // Memory statistics
            AnimationMemoryStats memoryStats;
            
            // Frame statistics
            uint32_t animatedCharacterCount = 0;
            uint32_t activeBoneCount = 0;
            uint32_t activeAnimationCount = 0;
            uint32_t activeIKSolverCount = 0;
            uint32_t activeMorphTargetCount = 0;
            
            // Performance metrics
            double frameTimeMs = 0.0;
            double animationCpuUsagePercent = 0.0;
            uint32_t framesSinceLastReset = 0;
        };

        /**
         * @brief Animation validation issue types
         */
        enum class AnimationValidationIssueType {
            Warning,
            Error,
            Performance
        };

        /**
         * @brief Animation validation issue
         */
        struct AnimationValidationIssue {
            AnimationValidationIssueType type;
            std::string category;
            std::string description;
            std::string suggestion;
            float severity = 0.0f; // 0.0 = low, 1.0 = critical
        };

        /**
         * @brief Animation validation report
         */
        struct AnimationValidationReport {
            std::vector<AnimationValidationIssue> issues;
            uint32_t warningCount = 0;
            uint32_t errorCount = 0;
            uint32_t performanceIssueCount = 0;
            float overallScore = 1.0f; // 0.0 = poor, 1.0 = excellent
            
            void CalculateCounts();
        };

        /**
         * @brief High-precision timer for performance measurement
         */
        class AnimationTimer {
        public:
            AnimationTimer();
            
            void Start();
            void Stop();
            double GetElapsedMs() const;
            double GetElapsedMicroseconds() const;
            bool IsRunning() const;
            
        private:
            std::chrono::high_resolution_clock::time_point m_startTime;
            std::chrono::high_resolution_clock::time_point m_endTime;
            bool m_isRunning = false;
        };

        /**
         * @brief RAII timer for automatic timing measurement
         */
        class ScopedAnimationTimer {
        public:
            explicit ScopedAnimationTimer(AnimationTimingData& timingData);
            ~ScopedAnimationTimer();
            
        private:
            AnimationTimingData& m_timingData;
            AnimationTimer m_timer;
        };

        /**
         * @brief Animation system performance profiler
         * 
         * Provides comprehensive performance analysis, memory usage tracking,
         * and validation tools for the animation system.
         */
        class AnimationProfiler {
        public:
            AnimationProfiler();
            ~AnimationProfiler();

            // Initialization
            bool Initialize();
            void Shutdown();

            // Profiling control
            void StartProfiling();
            void StopProfiling();
            void PauseProfiling();
            void ResumeProfiling();
            bool IsProfilingActive() const;

            // Frame timing
            void BeginFrame();
            void EndFrame();
            void ResetFrameStats();

            // Operation timing
            void BeginOperation(const std::string& operationName);
            void EndOperation(const std::string& operationName);
            AnimationTimingData GetOperationTiming(const std::string& operationName) const;

            // Memory analysis
            void UpdateMemoryStats(const AnimationController& controller);
            void UpdateMemoryStats(const AnimationSkeleton& skeleton);
            AnimationMemoryStats GetMemoryStats() const;

            // Performance statistics
            AnimationPerformanceStats GetPerformanceStats() const;
            void ResetPerformanceStats();

            // Validation and analysis
            AnimationValidationReport ValidateAnimationController(const AnimationController& controller);
            AnimationValidationReport ValidateAnimationSkeleton(const AnimationSkeleton& skeleton);
            AnimationValidationReport ValidateStateMachine(const AnimationStateMachine& stateMachine);
            AnimationValidationReport ValidateBlendTree(const BlendTree& blendTree);

            // Issue detection
            std::vector<AnimationValidationIssue> DetectPerformanceIssues(const AnimationPerformanceStats& stats);
            std::vector<AnimationValidationIssue> DetectMemoryIssues(const AnimationMemoryStats& memoryStats);
            std::vector<AnimationValidationIssue> DetectAnimationIssues(const AnimationController& controller);

            // Reporting
            std::string GeneratePerformanceReport() const;
            std::string GenerateMemoryReport() const;
            std::string GenerateValidationReport(const AnimationValidationReport& report) const;
            void ExportPerformanceData(const std::string& filename) const;

            // Configuration
            void SetProfilingEnabled(bool enabled);
            void SetMemoryTrackingEnabled(bool enabled);
            void SetValidationEnabled(bool enabled);
            void SetPerformanceThresholds(double maxFrameTimeMs, double maxOperationTimeMs);

            // Real-time monitoring
            void EnableRealTimeMonitoring(bool enabled);
            bool IsRealTimeMonitoringEnabled() const;
            void SetMonitoringCallback(std::function<void(const AnimationPerformanceStats&)> callback);

        private:
            bool m_isProfilingActive = false;
            bool m_isPaused = false;
            bool m_memoryTrackingEnabled = true;
            bool m_validationEnabled = true;
            bool m_realTimeMonitoringEnabled = false;

            // Performance data
            AnimationPerformanceStats m_performanceStats;
            std::unordered_map<std::string, AnimationTimingData> m_operationTimings;
            std::unordered_map<std::string, AnimationTimer> m_activeTimers;

            // Frame timing
            AnimationTimer m_frameTimer;
            std::vector<double> m_frameTimeHistory;
            static const size_t MAX_FRAME_HISTORY = 60;

            // Performance thresholds
            double m_maxFrameTimeMs = 16.67; // 60 FPS
            double m_maxOperationTimeMs = 1.0;

            // Monitoring callback
            std::function<void(const AnimationPerformanceStats&)> m_monitoringCallback;

            // Helper methods
            void UpdateFrameStats();
            void ValidatePerformanceThresholds();
            size_t CalculateSkeletonMemoryUsage(const AnimationSkeleton& skeleton) const;
            size_t CalculateControllerMemoryUsage(const AnimationController& controller) const;
            void AddValidationIssue(AnimationValidationReport& report, AnimationValidationIssueType type,
                                  const std::string& category, const std::string& description,
                                  const std::string& suggestion, float severity) const;
        };

        /**
         * @brief Global animation profiler instance
         */
        class AnimationProfilerManager {
        public:
            static AnimationProfiler& GetInstance();
            static void Initialize();
            static void Shutdown();

        private:
            static std::unique_ptr<AnimationProfiler> s_instance;
        };

        // Convenience macros for profiling
        #define GAMEENGINE_PROFILE_ANIMATION_OPERATION(name) \
            ScopedAnimationTimer timer(AnimationProfilerManager::GetInstance().GetOperationTiming(name))

        #define GAMEENGINE_PROFILE_ANIMATION_FRAME() \
            AnimationProfilerManager::GetInstance().BeginFrame(); \
            struct FrameEndGuard { ~FrameEndGuard() { AnimationProfilerManager::GetInstance().EndFrame(); } } frameGuard

    } // namespace Animation
} // namespace GameEngine