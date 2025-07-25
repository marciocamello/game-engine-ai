#include "Resource/ResourceUsageTracker.h"
#include "Core/Logger.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace GameEngine {

    ResourceUsageTracker::ResourceUsageTracker() {
        LOG_INFO("ResourceUsageTracker initialized");
    }

    ResourceUsageTracker::~ResourceUsageTracker() {
        LOG_INFO("ResourceUsageTracker destroyed");
    }

    void ResourceUsageTracker::TrackResourceLoad(const std::string& path, const std::string& type, size_t memoryUsage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto now = std::chrono::steady_clock::now();
        
        ResourceUsageInfo info;
        info.resourcePath = path;
        info.resourceType = type;
        info.memoryUsage = memoryUsage;
        info.accessCount = 1; // First access is the load
        info.lastAccessTime = now;
        info.loadTime = now;
        
        m_resourceUsage[path] = info;
        InvalidateStatsCache();
        
        LOG_DEBUG("Tracked resource load: " + path + " (" + type + ", " + 
                 std::to_string(memoryUsage / 1024) + " KB)");
        
        // Cleanup if we have too many tracked resources
        if (m_resourceUsage.size() > m_maxTrackedResources) {
            CleanupOldEntries();
        }
    }

    void ResourceUsageTracker::TrackResourceAccess(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_resourceUsage.find(path);
        if (it != m_resourceUsage.end()) {
            it->second.accessCount++;
            it->second.lastAccessTime = std::chrono::steady_clock::now();
            InvalidateStatsCache();
            
            LOG_DEBUG("Tracked resource access: " + path + " (access count: " + 
                     std::to_string(it->second.accessCount) + ")");
        }
    }

    void ResourceUsageTracker::TrackResourceUnload(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_resourceUsage.find(path);
        if (it != m_resourceUsage.end()) {
            LOG_DEBUG("Tracked resource unload: " + path);
            m_resourceUsage.erase(it);
            InvalidateStatsCache();
        }
    }

    void ResourceUsageTracker::UpdateResourceMemoryUsage(const std::string& path, size_t newMemoryUsage) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_resourceUsage.find(path);
        if (it != m_resourceUsage.end()) {
            size_t oldMemory = it->second.memoryUsage;
            it->second.memoryUsage = newMemoryUsage;
            InvalidateStatsCache();
            
            LOG_DEBUG("Updated resource memory usage: " + path + " (" + 
                     std::to_string(oldMemory / 1024) + " KB -> " + 
                     std::to_string(newMemoryUsage / 1024) + " KB)");
        }
    }

    ResourceUsageTracker::UsageStatistics ResourceUsageTracker::GetUsageStatistics() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (!m_statsCacheValid) {
            UpdateCachedStats();
        }
        
        return m_cachedStats;
    }

    std::vector<std::string> ResourceUsageTracker::GetLRUCandidates(size_t maxCandidates) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::pair<std::string, double>> candidates;
        
        for (const auto& pair : m_resourceUsage) {
            double score = pair.second.GetUsageScore();
            candidates.emplace_back(pair.first, score);
        }
        
        // Sort by usage score (higher score = better candidate for eviction)
        std::sort(candidates.begin(), candidates.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::vector<std::string> result;
        size_t count = std::min(maxCandidates, candidates.size());
        for (size_t i = 0; i < count; ++i) {
            result.push_back(candidates[i].first);
        }
        
        return result;
    }

    std::vector<std::string> ResourceUsageTracker::GetMemoryHeavyResources(size_t maxResources) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::vector<std::pair<std::string, size_t>> resources;
        
        for (const auto& pair : m_resourceUsage) {
            resources.emplace_back(pair.first, pair.second.memoryUsage);
        }
        
        // Sort by memory usage (descending)
        std::sort(resources.begin(), resources.end(), 
                 [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::vector<std::string> result;
        size_t count = std::min(maxResources, resources.size());
        for (size_t i = 0; i < count; ++i) {
            result.push_back(resources[i].first);
        }
        
        return result;
    }

    size_t ResourceUsageTracker::GetTotalMemoryUsage() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        size_t total = 0;
        for (const auto& pair : m_resourceUsage) {
            total += pair.second.memoryUsage;
        }
        
        return total;
    }

    std::vector<std::string> ResourceUsageTracker::GetEvictionCandidates(size_t targetMemoryReduction) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto lruCandidates = GetLRUCandidates(m_resourceUsage.size());
        
        std::vector<std::string> result;
        size_t memoryFreed = 0;
        
        for (const auto& path : lruCandidates) {
            auto it = m_resourceUsage.find(path);
            if (it != m_resourceUsage.end()) {
                result.push_back(path);
                memoryFreed += it->second.memoryUsage;
                
                if (memoryFreed >= targetMemoryReduction) {
                    break;
                }
            }
        }
        
        return result;
    }

    void ResourceUsageTracker::LogUsageStatistics() const {
        auto stats = GetUsageStatistics();
        
        LOG_INFO("=== Resource Usage Statistics ===");
        LOG_INFO("Total Resources: " + std::to_string(stats.totalResources));
        LOG_INFO("Total Memory Usage: " + std::to_string(stats.totalMemoryUsage / (1024 * 1024)) + " MB");
        LOG_INFO("Total Access Count: " + std::to_string(stats.totalAccessCount));
        
        LOG_INFO("Resources by Type:");
        for (const auto& pair : stats.resourcesByType) {
            LOG_INFO("  " + pair.first + ": " + std::to_string(pair.second));
        }
        
        LOG_INFO("Memory by Type:");
        for (const auto& pair : stats.memoryByType) {
            LOG_INFO("  " + pair.first + ": " + std::to_string(pair.second / (1024 * 1024)) + " MB");
        }
        
        if (!stats.largestResources.empty()) {
            LOG_INFO("Largest Resources:");
            for (size_t i = 0; i < std::min(size_t(5), stats.largestResources.size()); ++i) {
                const auto& info = stats.largestResources[i];
                LOG_INFO("  " + info.resourcePath + " (" + info.resourceType + "): " + 
                        std::to_string(info.memoryUsage / 1024) + " KB");
            }
        }
    }

    void ResourceUsageTracker::LogResourceDetails(const std::string& path) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto it = m_resourceUsage.find(path);
        if (it != m_resourceUsage.end()) {
            const auto& info = it->second;
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLoad = std::chrono::duration_cast<std::chrono::seconds>(now - info.loadTime).count();
            auto timeSinceAccess = std::chrono::duration_cast<std::chrono::seconds>(now - info.lastAccessTime).count();
            
            LOG_INFO("=== Resource Details: " + path + " ===");
            LOG_INFO("Type: " + info.resourceType);
            LOG_INFO("Memory Usage: " + std::to_string(info.memoryUsage / 1024) + " KB");
            LOG_INFO("Access Count: " + std::to_string(info.accessCount));
            LOG_INFO("Time Since Load: " + std::to_string(timeSinceLoad) + " seconds");
            LOG_INFO("Time Since Last Access: " + std::to_string(timeSinceAccess) + " seconds");
            LOG_INFO("Usage Score: " + std::to_string(info.GetUsageScore()));
        } else {
            LOG_WARNING("Resource not found in usage tracker: " + path);
        }
    }

    void ResourceUsageTracker::ExportUsageReport(const std::string& filePath) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        std::ofstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open file for usage report: " + filePath);
            return;
        }
        
        auto stats = GetUsageStatistics();
        
        file << "Resource Usage Report\n";
        file << "Generated: " << std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count() << "\n\n";
        
        file << "Summary:\n";
        file << "Total Resources: " << stats.totalResources << "\n";
        file << "Total Memory Usage: " << (stats.totalMemoryUsage / (1024 * 1024)) << " MB\n";
        file << "Total Access Count: " << stats.totalAccessCount << "\n\n";
        
        file << "Detailed Resource List:\n";
        file << std::left << std::setw(50) << "Path" 
             << std::setw(15) << "Type" 
             << std::setw(15) << "Memory (KB)" 
             << std::setw(15) << "Access Count" 
             << std::setw(15) << "Usage Score" << "\n";
        file << std::string(110, '-') << "\n";
        
        for (const auto& pair : m_resourceUsage) {
            const auto& info = pair.second;
            file << std::left << std::setw(50) << info.resourcePath
                 << std::setw(15) << info.resourceType
                 << std::setw(15) << (info.memoryUsage / 1024)
                 << std::setw(15) << info.accessCount
                 << std::setw(15) << std::fixed << std::setprecision(3) << info.GetUsageScore() << "\n";
        }
        
        file.close();
        LOG_INFO("Usage report exported to: " + filePath);
    }

    void ResourceUsageTracker::SetMemoryPressureThreshold(size_t thresholdBytes) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_memoryPressureThreshold = thresholdBytes;
        LOG_INFO("Memory pressure threshold set to: " + std::to_string(thresholdBytes / (1024 * 1024)) + " MB");
    }

    void ResourceUsageTracker::SetMaxTrackedResources(size_t maxResources) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_maxTrackedResources = maxResources;
        LOG_INFO("Max tracked resources set to: " + std::to_string(maxResources));
    }

    void ResourceUsageTracker::ClearStatistics() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_resourceUsage.clear();
        InvalidateStatsCache();
        LOG_INFO("Resource usage statistics cleared");
    }

    void ResourceUsageTracker::RemoveOldEntries(std::chrono::seconds maxAge) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto now = std::chrono::steady_clock::now();
        size_t removedCount = 0;
        
        auto it = m_resourceUsage.begin();
        while (it != m_resourceUsage.end()) {
            auto age = std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastAccessTime);
            if (age > maxAge) {
                it = m_resourceUsage.erase(it);
                removedCount++;
            } else {
                ++it;
            }
        }
        
        if (removedCount > 0) {
            InvalidateStatsCache();
            LOG_INFO("Removed " + std::to_string(removedCount) + " old resource usage entries");
        }
    }

    void ResourceUsageTracker::InvalidateStatsCache() {
        m_statsCacheValid = false;
    }

    void ResourceUsageTracker::UpdateCachedStats() const {
        m_cachedStats = UsageStatistics{};
        
        std::vector<ResourceUsageInfo> allResources;
        
        for (const auto& pair : m_resourceUsage) {
            const auto& info = pair.second;
            
            m_cachedStats.totalResources++;
            m_cachedStats.totalMemoryUsage += info.memoryUsage;
            m_cachedStats.totalAccessCount += info.accessCount;
            
            m_cachedStats.resourcesByType[info.resourceType]++;
            m_cachedStats.memoryByType[info.resourceType] += info.memoryUsage;
            
            allResources.push_back(info);
        }
        
        // Sort for top lists
        std::sort(allResources.begin(), allResources.end(), 
                 [](const auto& a, const auto& b) { return a.accessCount > b.accessCount; });
        
        size_t topCount = std::min(size_t(10), allResources.size());
        for (size_t i = 0; i < topCount; ++i) {
            m_cachedStats.mostUsedResources.push_back(allResources[i]);
        }
        
        // Sort by memory usage
        std::sort(allResources.begin(), allResources.end(), 
                 [](const auto& a, const auto& b) { return a.memoryUsage > b.memoryUsage; });
        
        m_cachedStats.largestResources.clear();
        for (size_t i = 0; i < topCount; ++i) {
            m_cachedStats.largestResources.push_back(allResources[i]);
        }
        
        // Sort by usage score for LRU
        std::sort(allResources.begin(), allResources.end(), 
                 [](const auto& a, const auto& b) { return a.GetUsageScore() > b.GetUsageScore(); });
        
        m_cachedStats.leastUsedResources.clear();
        for (size_t i = 0; i < topCount; ++i) {
            m_cachedStats.leastUsedResources.push_back(allResources[i]);
        }
        
        m_statsCacheValid = true;
    }

    void ResourceUsageTracker::CleanupOldEntries() {
        // Remove the oldest 10% of entries when we hit the limit
        size_t targetRemoval = m_resourceUsage.size() / 10;
        if (targetRemoval == 0) targetRemoval = 1;
        
        std::vector<std::pair<std::string, std::chrono::steady_clock::time_point>> entries;
        for (const auto& pair : m_resourceUsage) {
            entries.emplace_back(pair.first, pair.second.lastAccessTime);
        }
        
        std::sort(entries.begin(), entries.end(), 
                 [](const auto& a, const auto& b) { return a.second < b.second; });
        
        for (size_t i = 0; i < targetRemoval && i < entries.size(); ++i) {
            m_resourceUsage.erase(entries[i].first);
        }
        
        LOG_DEBUG("Cleaned up " + std::to_string(targetRemoval) + " old resource usage entries");
    }

    // Global instance implementation
    std::unique_ptr<ResourceUsageTracker> GlobalResourceTracker::s_instance;
    std::mutex GlobalResourceTracker::s_instanceMutex;

    ResourceUsageTracker& GlobalResourceTracker::GetInstance() {
        std::lock_guard<std::mutex> lock(s_instanceMutex);
        if (!s_instance) {
            s_instance = std::make_unique<ResourceUsageTracker>();
        }
        return *s_instance;
    }

} // namespace GameEngine