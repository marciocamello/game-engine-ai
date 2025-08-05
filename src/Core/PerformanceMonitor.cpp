#include "Core/PerformanceMonitor.h"
#include "Core/Logger.h"
#include <algorithm>
#include <numeric>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

namespace GameEngine {

    PerformanceMonitor::PerformanceMonitor() {
        m_frameTimes.reserve(MAX_FRAME_SAMPLES);
        m_lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    PerformanceMonitor::~PerformanceMonitor() = default;

    void PerformanceMonitor::BeginFrame() {
        m_frameStart = std::chrono::high_resolution_clock::now();
    }

    void PerformanceMonitor::EndFrame() {
        auto frameEnd = std::chrono::high_resolution_clock::now();
        
        if (!m_firstFrame) {
            auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - m_lastFrameTime);
            float frameTimeMs = frameDuration.count() / 1000.0f;
            
            // Add frame time to samples
            m_frameTimes.push_back(frameTimeMs);
            if (m_frameTimes.size() > MAX_FRAME_SAMPLES) {
                m_frameTimes.erase(m_frameTimes.begin());
            }
            
            UpdateStats();
        } else {
            m_firstFrame = false;
        }
        
        m_lastFrameTime = frameEnd;
        
        // Update memory usage every 60 frames (once per second at 60 FPS)
        static int frameCounter = 0;
        if (++frameCounter >= 60) {
            UpdateMemoryUsage();
            frameCounter = 0;
        }
    }

    void PerformanceMonitor::UpdateStats() {
        if (m_frameTimes.empty()) return;
        
        // Calculate current FPS
        float currentFrameTime = m_frameTimes.back();
        m_frameStats.frameTime = currentFrameTime;
        m_frameStats.fps = (currentFrameTime > 0.0f) ? (1000.0f / currentFrameTime) : 0.0f;
        
        // Calculate average FPS over sample window
        float averageFrameTime = std::accumulate(m_frameTimes.begin(), m_frameTimes.end(), 0.0f) / m_frameTimes.size();
        m_frameStats.averageFPS = (averageFrameTime > 0.0f) ? (1000.0f / averageFrameTime) : 0.0f;
        
        // Calculate min/max FPS
        auto minMaxFrameTime = std::minmax_element(m_frameTimes.begin(), m_frameTimes.end());
        m_frameStats.maxFPS = (*minMaxFrameTime.first > 0.0f) ? (1000.0f / *minMaxFrameTime.first) : 0.0f;
        m_frameStats.minFPS = (*minMaxFrameTime.second > 0.0f) ? (1000.0f / *minMaxFrameTime.second) : 0.0f;
        
        // Log performance warnings
        if (m_frameStats.averageFPS < 60.0f && m_frameTimes.size() >= MAX_FRAME_SAMPLES) {
            static auto lastWarning = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastWarning).count() >= 5) {
                LOG_WARNING("Performance below target: " + std::to_string(m_frameStats.averageFPS) + " FPS (target: 60+ FPS)");
                lastWarning = now;
            }
        }
    }

    void PerformanceMonitor::UpdateMemoryUsage() {
        m_frameStats.memoryUsageMB = GetProcessMemoryUsage();
        
        // Log memory warnings
        if (m_frameStats.memoryUsageMB > 200) {
            static auto lastMemoryWarning = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(now - lastMemoryWarning).count() >= 10) {
                LOG_WARNING("High memory usage: " + std::to_string(m_frameStats.memoryUsageMB) + " MB (target: <200 MB)");
                lastMemoryWarning = now;
            }
        }
    }

    size_t PerformanceMonitor::GetProcessMemoryUsage() const {
#ifdef _WIN32
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize / (1024 * 1024); // Convert to MB
        }
#endif
        return 0;
    }

    void PerformanceMonitor::LogPerformanceReport() const {
        LOG_INFO("========================================");
        LOG_INFO("PERFORMANCE REPORT");
        LOG_INFO("========================================");
        LOG_INFO("Current FPS: " + std::to_string(m_frameStats.fps));
        LOG_INFO("Average FPS: " + std::to_string(m_frameStats.averageFPS));
        LOG_INFO("Min FPS: " + std::to_string(m_frameStats.minFPS));
        LOG_INFO("Max FPS: " + std::to_string(m_frameStats.maxFPS));
        LOG_INFO("Frame Time: " + std::to_string(m_frameStats.frameTime) + " ms");
        LOG_INFO("Memory Usage: " + std::to_string(m_frameStats.memoryUsageMB) + " MB");
        LOG_INFO("Performance Target: " + std::string(IsPerformanceTarget() ? "MET" : "NOT MET"));
        LOG_INFO("========================================");
    }

}