#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <mutex>

namespace GameEngine {

    /**
     * @brief Resource pooling system for efficient memory management
     * 
     * Prevents memory leaks by managing resource lifecycles and providing
     * automatic cleanup for unused resources.
     */
    template<typename T>
    class ResourcePool {
    public:
        using ResourcePtr = std::shared_ptr<T>;

        ResourcePool() = default;
        ~ResourcePool() = default;

        // Get or create resource
        template<typename... Args>
        ResourcePtr GetOrCreate(const std::string& key, Args&&... args) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_resources.find(key);
            if (it != m_resources.end()) {
                if (auto resource = it->second.lock()) {
                    return resource;
                } else {
                    // Resource expired, remove from map
                    m_resources.erase(it);
                }
            }
            
            // Create new resource
            auto resource = std::make_shared<T>(std::forward<Args>(args)...);
            m_resources[key] = resource;
            return resource;
        }

        // Get existing resource (returns nullptr if not found)
        ResourcePtr Get(const std::string& key) {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_resources.find(key);
            if (it != m_resources.end()) {
                if (auto resource = it->second.lock()) {
                    return resource;
                } else {
                    // Resource expired, remove from map
                    m_resources.erase(it);
                }
            }
            
            return nullptr;
        }

        // Force cleanup of expired resources
        void CleanupExpired() {
            std::lock_guard<std::mutex> lock(m_mutex);
            
            auto it = m_resources.begin();
            while (it != m_resources.end()) {
                if (it->second.expired()) {
                    it = m_resources.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // Get resource count
        size_t GetResourceCount() const {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_resources.size();
        }

        // Clear all resources
        void Clear() {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_resources.clear();
        }

    private:
        std::unordered_map<std::string, std::weak_ptr<T>> m_resources;
        mutable std::mutex m_mutex;
    };

}