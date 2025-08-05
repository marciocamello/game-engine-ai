#pragma once

#include "Core/Math.h"
#include <chrono>
#include <vector>
#include <string>

namespace GameEngine {

    /**
     * @brief Performance monitoring system for tracking FPS and frame times
     * 
     * Provides real-time performance metrics to ensure 60+ FPS target is maintained.
     * Includes memory usage tracking and performance warnings.
     */
    class PerformanceMonitor {
    public:
        struct FrameStats {
            float fps = 0.0f;
            float frameTime = 0.0f;
            float averageFPS = 0.0f;
            float minFPS = 0.0f;
            float maxFPS = 0.0f;
            size_t memoryUsageMB = 0;
        };

        PerformanceMonitor();
        ~PerformanceMonitor();

        void BeginFrame();
        void EndFrame();
        
        const FrameStats& GetFrameStats() const { return m_frameStats; }
        
        bool IsPerformanceTarget() const { return m_frameStats.averageFPS >= 60.0f; }
        void LogPerformanceReport() const;
        
        // Memory tracking
        void UpdateMemoryUsage();
        size_t GetMemoryUsageMB() const { return m_frameStats.memoryUsageMB; }

    private:
        void UpdateStats();
        size_t GetProcessMemoryUsage() const;

        std::chrono::high_resolution_clock::time_point m_frameStart;
        std::chrono::high_resolution_clock::time_point m_lastFrameTime;
        
        std::vector<float> m_frameTimes;
        static constexpr size_t MAX_FRAME_SAMPLES = 120; // 2 seconds at 60 FPS
        
        FrameStats m_frameStats;
        bool m_firstFrame = true;
    };

}