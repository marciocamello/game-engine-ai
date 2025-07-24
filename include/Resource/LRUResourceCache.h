#pragma once

#include <unordered_map>
#include <list>
#include <memory>
#include <string>
#include <chrono>
#include <mutex>

namespace GameEngine {

    class Resource;

    // LRU (Least Recently Used) cache for automatic resource cleanup
    template<typename T>
    class LRUResourceCache {
    public:
        struct CacheEntry {
            std::shared_ptr<T> resource;
            std::chrono::steady_clock::time_point lastAccess;
            size_t accessCount = 0;
            bool isPinned = false; // Pinned resources are never evicted
            
            CacheEntry(std::shared_ptr<T> res) 
                : resource(res), lastAccess(std::chrono::steady_clock::now()) {}
        };
        
        using CacheIterator = typename std::list<std::pair<std::string, CacheEntry>>::iterator;

    public:
        LRUResourceCache(size_t maxSize = 100, size_t maxMemory = 256 * 1024 * 1024);
        ~LRUResourceCache();

        // Cache operations
        std::shared_ptr<T> Get(const std::string& key);
        void Put(const std::string& key, std::shared_ptr<T> resource);
        void Remove(const std::string& key);
        void Clear();
        
        // Cache management
        void SetMaxSize(size_t maxSize) { m_maxSize = maxSize; }
        void SetMaxMemory(size_t maxMemory) { m_maxMemory = maxMemory; }
        void EvictLRU(size_t count = 1);
        void EvictByMemory(size_t targetMemory);
        void EvictOlderThan(std::chrono::seconds maxAge);
        
        // Pinning (prevents eviction)
        void Pin(const std::string& key);
        void Unpin(const std::string& key);
        bool IsPinned(const std::string& key) const;
        
        // Statistics
        size_t GetSize() const { return m_cacheList.size(); }
        size_t GetMemoryUsage() const;
        float GetHitRatio() const;
        void ResetStatistics();
        
        // Cache state
        bool Contains(const std::string& key) const;
        std::vector<std::string> GetKeys() const;
        std::vector<std::pair<std::string, size_t>> GetMemoryUsageByResource() const;
        
        // Maintenance
        void Cleanup(); // Remove expired weak references
        void Optimize(); // Reorganize for better performance

    private:
        mutable std::mutex m_cacheMutex;
        
        // LRU implementation using list + hash map
        std::list<std::pair<std::string, CacheEntry>> m_cacheList;
        std::unordered_map<std::string, CacheIterator> m_cacheMap;
        
        size_t m_maxSize;
        size_t m_maxMemory;
        
        // Statistics
        mutable size_t m_hits = 0;
        mutable size_t m_misses = 0;
        mutable size_t m_evictions = 0;
        
        // Internal methods
        void MoveToFront(CacheIterator it);
        void EvictLRUInternal();
        bool ShouldEvict() const;
        size_t CalculateMemoryUsage() const;
    };

    // Template implementation
    template<typename T>
    LRUResourceCache<T>::LRUResourceCache(size_t maxSize, size_t maxMemory)
        : m_maxSize(maxSize), m_maxMemory(maxMemory) {
    }

    template<typename T>
    LRUResourceCache<T>::~LRUResourceCache() {
        Clear();
    }

    template<typename T>
    std::shared_ptr<T> LRUResourceCache<T>::Get(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto mapIt = m_cacheMap.find(key);
        if (mapIt != m_cacheMap.end()) {
            // Cache hit - move to front and update access info
            auto listIt = mapIt->second;
            listIt->second.lastAccess = std::chrono::steady_clock::now();
            listIt->second.accessCount++;
            
            MoveToFront(listIt);
            m_hits++;
            
            return listIt->second.resource;
        }
        
        // Cache miss
        m_misses++;
        return nullptr;
    }

    template<typename T>
    void LRUResourceCache<T>::Put(const std::string& key, std::shared_ptr<T> resource) {
        if (!resource) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        // Check if key already exists
        auto mapIt = m_cacheMap.find(key);
        if (mapIt != m_cacheMap.end()) {
            // Update existing entry
            auto listIt = mapIt->second;
            listIt->second.resource = resource;
            listIt->second.lastAccess = std::chrono::steady_clock::now();
            listIt->second.accessCount++;
            
            MoveToFront(listIt);
            return;
        }
        
        // Add new entry
        CacheEntry entry(resource);
        m_cacheList.emplace_front(key, std::move(entry));
        m_cacheMap[key] = m_cacheList.begin();
        
        // Check if we need to evict
        while (ShouldEvict()) {
            EvictLRUInternal();
        }
    }

    template<typename T>
    void LRUResourceCache<T>::Remove(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto mapIt = m_cacheMap.find(key);
        if (mapIt != m_cacheMap.end()) {
            m_cacheList.erase(mapIt->second);
            m_cacheMap.erase(mapIt);
        }
    }

    template<typename T>
    void LRUResourceCache<T>::Clear() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        m_cacheList.clear();
        m_cacheMap.clear();
        ResetStatistics();
    }

    template<typename T>
    void LRUResourceCache<T>::EvictLRU(size_t count) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        for (size_t i = 0; i < count && !m_cacheList.empty(); ++i) {
            EvictLRUInternal();
        }
    }

    template<typename T>
    void LRUResourceCache<T>::EvictByMemory(size_t targetMemory) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        size_t currentMemory = CalculateMemoryUsage();
        
        while (currentMemory > targetMemory && !m_cacheList.empty()) {
            size_t beforeMemory = currentMemory;
            EvictLRUInternal();
            currentMemory = CalculateMemoryUsage();
            
            // Prevent infinite loop if eviction doesn't reduce memory
            if (currentMemory >= beforeMemory) {
                break;
            }
        }
    }

    template<typename T>
    void LRUResourceCache<T>::EvictOlderThan(std::chrono::seconds maxAge) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto now = std::chrono::steady_clock::now();
        auto cutoffTime = now - maxAge;
        
        for (auto it = m_cacheList.rbegin(); it != m_cacheList.rend();) {
            if (it->second.lastAccess < cutoffTime && !it->second.isPinned) {
                // Convert reverse iterator to forward iterator for erase
                auto forwardIt = std::next(it).base();
                m_cacheMap.erase(it->first);
                it = std::make_reverse_iterator(m_cacheList.erase(forwardIt));
                m_evictions++;
            } else {
                ++it;
            }
        }
    }

    template<typename T>
    void LRUResourceCache<T>::Pin(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto mapIt = m_cacheMap.find(key);
        if (mapIt != m_cacheMap.end()) {
            mapIt->second->second.isPinned = true;
        }
    }

    template<typename T>
    void LRUResourceCache<T>::Unpin(const std::string& key) {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto mapIt = m_cacheMap.find(key);
        if (mapIt != m_cacheMap.end()) {
            mapIt->second->second.isPinned = false;
        }
    }

    template<typename T>
    bool LRUResourceCache<T>::IsPinned(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        auto mapIt = m_cacheMap.find(key);
        if (mapIt != m_cacheMap.end()) {
            return mapIt->second->second.isPinned;
        }
        return false;
    }

    template<typename T>
    size_t LRUResourceCache<T>::GetMemoryUsage() const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return CalculateMemoryUsage();
    }

    template<typename T>
    float LRUResourceCache<T>::GetHitRatio() const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        size_t totalAccesses = m_hits + m_misses;
        if (totalAccesses == 0) {
            return 0.0f;
        }
        
        return static_cast<float>(m_hits) / totalAccesses;
    }

    template<typename T>
    void LRUResourceCache<T>::ResetStatistics() {
        m_hits = 0;
        m_misses = 0;
        m_evictions = 0;
    }

    template<typename T>
    bool LRUResourceCache<T>::Contains(const std::string& key) const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        return m_cacheMap.find(key) != m_cacheMap.end();
    }

    template<typename T>
    std::vector<std::string> LRUResourceCache<T>::GetKeys() const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        std::vector<std::string> keys;
        keys.reserve(m_cacheList.size());
        
        for (const auto& pair : m_cacheList) {
            keys.push_back(pair.first);
        }
        
        return keys;
    }

    template<typename T>
    std::vector<std::pair<std::string, size_t>> LRUResourceCache<T>::GetMemoryUsageByResource() const {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        std::vector<std::pair<std::string, size_t>> usage;
        usage.reserve(m_cacheList.size());
        
        for (const auto& pair : m_cacheList) {
            size_t memUsage = 0;
            if (pair.second.resource) {
                memUsage = pair.second.resource->GetMemoryUsage();
            }
            usage.emplace_back(pair.first, memUsage);
        }
        
        return usage;
    }

    template<typename T>
    void LRUResourceCache<T>::Cleanup() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        // Remove entries with expired resources
        for (auto it = m_cacheList.begin(); it != m_cacheList.end();) {
            if (!it->second.resource) {
                m_cacheMap.erase(it->first);
                it = m_cacheList.erase(it);
            } else {
                ++it;
            }
        }
    }

    template<typename T>
    void LRUResourceCache<T>::Optimize() {
        std::lock_guard<std::mutex> lock(m_cacheMutex);
        
        // Sort by access frequency (most accessed first)
        m_cacheList.sort([](const auto& a, const auto& b) {
            return a.second.accessCount > b.second.accessCount;
        });
        
        // Rebuild the map with new iterators
        m_cacheMap.clear();
        for (auto it = m_cacheList.begin(); it != m_cacheList.end(); ++it) {
            m_cacheMap[it->first] = it;
        }
    }

    template<typename T>
    void LRUResourceCache<T>::MoveToFront(CacheIterator it) {
        if (it != m_cacheList.begin()) {
            // Move element to front
            auto entry = std::move(*it);
            m_cacheList.erase(it);
            m_cacheList.push_front(std::move(entry));
            
            // Update map
            m_cacheMap[m_cacheList.front().first] = m_cacheList.begin();
        }
    }

    template<typename T>
    void LRUResourceCache<T>::EvictLRUInternal() {
        if (m_cacheList.empty()) {
            return;
        }
        
        // Find the least recently used non-pinned resource
        auto it = m_cacheList.rbegin();
        while (it != m_cacheList.rend() && it->second.isPinned) {
            ++it;
        }
        
        if (it != m_cacheList.rend()) {
            // Convert reverse iterator to forward iterator for erase
            auto forwardIt = std::next(it).base();
            m_cacheMap.erase(it->first);
            m_cacheList.erase(forwardIt);
            m_evictions++;
        }
    }

    template<typename T>
    bool LRUResourceCache<T>::ShouldEvict() const {
        return m_cacheList.size() > m_maxSize || CalculateMemoryUsage() > m_maxMemory;
    }

    template<typename T>
    size_t LRUResourceCache<T>::CalculateMemoryUsage() const {
        size_t totalMemory = 0;
        
        for (const auto& pair : m_cacheList) {
            if (pair.second.resource) {
                totalMemory += pair.second.resource->GetMemoryUsage();
            }
        }
        
        return totalMemory;
    }

} // namespace GameEngine