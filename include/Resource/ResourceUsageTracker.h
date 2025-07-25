#pragma once

#include <unordered_map>
#include <string>
#include <chrono>
#include <memory>
#include <mutex>

namespace GameEngine {

    /**
     * @brief Tracks resource usage statistics for optimization and debugging
     */
    class ResourceUsageTracker {
    public:
        struct ResourceUsageInfo {
            std::string resourcePath;
            std::string resourceType;
            size_t memoryUsage = 0;
            size_t accessCount = 0;
            std::chrono::steady_clock::time_point lastAccessTime;
            std::chrono::steady_clock::time_point loadTime;
            
            // Calculate usage score for LRU eviction
            double GetUsageScore() const {
                auto now = std::chrono::steady_clock::now();
                auto timeSinceLastAccess = std::chrono::duration_cast<std::chrono::seconds>(now - lastAccessTime).count();
                auto timeSinceLoad = std::chrono::duration_cast<std::chrono::seconds>(now - loadTime).count();
                
                // Higher score = more likely to be evicted
                // Factors: time since last access, memory usage, access frequency
                double timeScore = static_cast<double>(timeSinceLastAccess) / 3600.0; // Hours since last access
                double memoryScore = static_cast<double>(memoryUsage) / (1024 * 1024); // MB of memory
                double accessScore = accessCount > 0 ? 1.0 / static_cast<double>(accessCount) : 1.0;
                
                return timeScore * 0.5 + memoryScore * 0.3 + accessScore * 0.2;
            }
        };

        struct UsageStatistics {
            size_t totalResources = 0;
            size_t totalMemoryUsage = 0;
            size_t totalAccessCount = 0;
            std::unordered_map<std::string, size_t> resourcesByType;
            std::unordered_map<std::string, size_t> memoryByType;
            std::vector<ResourceUsageInfo> mostUsedResources;
            std::vector<ResourceUsageInfo> leastUsedResources;
            std::vector<ResourceUsageInfo> largestResources;
        };

    public:
        ResourceUsageTracker();
        ~ResourceUsageTracker();

        // Resource tracking
        void TrackResourceLoad(const std::string& path, const std::string& type, size_t memoryUsage);
        void TrackResourceAccess(const std::string& path);
        void TrackResourceUnload(const std::string& path);
        void UpdateResourceMemoryUsage(const std::string& path, size_t newMemoryUsage);

        // Statistics and reporting
        UsageStatistics GetUsageStatistics() const;
        std::vector<std::string> GetLRUCandidates(size_t maxCandidates = 10) const;
        std::vector<std::string> GetMemoryHeavyResources(size_t maxResources = 10) const;
        
        // Memory pressure management
        size_t GetTotalMemoryUsage() const;
        std::vector<std::string> GetEvictionCandidates(size_t targetMemoryReduction) const;
        
        // Debugging and logging
        void LogUsageStatistics() const;
        void LogResourceDetails(const std::string& path) const;
        void ExportUsageReport(const std::string& filePath) const;

        // Configuration
        void SetMemoryPressureThreshold(size_t thresholdBytes);
        void SetMaxTrackedResources(size_t maxResources);
        
        // Cleanup
        void ClearStatistics();
        void RemoveOldEntries(std::chrono::seconds maxAge);

    private:
        mutable std::mutex m_mutex;
        std::unordered_map<std::string, ResourceUsageInfo> m_resourceUsage;
        
        // Configuration
        size_t m_memoryPressureThreshold = 512 * 1024 * 1024; // 512 MB
        size_t m_maxTrackedResources = 1000;
        
        // Statistics
        mutable UsageStatistics m_cachedStats;
        mutable bool m_statsCacheValid = false;
        
        // Helper methods
        void InvalidateStatsCache();
        void UpdateCachedStats() const;
        void CleanupOldEntries();
    };

    /**
     * @brief Global resource usage tracker instance
     */
    class GlobalResourceTracker {
    public:
        static ResourceUsageTracker& GetInstance();
        
    private:
        static std::unique_ptr<ResourceUsageTracker> s_instance;
        static std::mutex s_instanceMutex;
    };

} // namespace GameEngine