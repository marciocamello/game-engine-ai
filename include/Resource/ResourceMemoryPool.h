#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstddef>

namespace GameEngine {

    // Memory pool for efficient resource allocation
    class ResourceMemoryPool {
    public:
        ResourceMemoryPool();
        ~ResourceMemoryPool();

        // Memory allocation
        void* Allocate(size_t size, size_t alignment = alignof(std::max_align_t));
        void Deallocate(void* ptr, size_t size);
        
        // Pool management
        void SetPoolSize(size_t poolSize);
        void SetChunkSize(size_t chunkSize);
        void PreallocatePool();
        void ShrinkToFit();
        void Clear();
        
        // Statistics
        size_t GetTotalAllocated() const { return m_totalAllocated; }
        size_t GetTotalPoolSize() const { return m_totalPoolSize; }
        size_t GetFragmentation() const;
        float GetUtilization() const;
        
        // Configuration
        void EnablePooling(bool enabled) { m_poolingEnabled = enabled; }
        void SetDefragmentationThreshold(float threshold) { m_defragThreshold = threshold; }
        
    private:
        struct MemoryChunk {
            void* data = nullptr;
            size_t size = 0;
            bool inUse = false;
            MemoryChunk* next = nullptr;
        };
        
        struct MemoryBlock {
            void* data = nullptr;
            size_t size = 0;
            std::vector<MemoryChunk> chunks;
        };
        
        std::vector<MemoryBlock> m_memoryBlocks;
        std::unordered_map<void*, MemoryChunk*> m_allocatedChunks;
        
        mutable std::mutex m_poolMutex;
        
        size_t m_poolSize = 64 * 1024 * 1024; // 64MB default
        size_t m_chunkSize = 1024; // 1KB default chunk size
        size_t m_totalAllocated = 0;
        size_t m_totalPoolSize = 0;
        float m_defragThreshold = 0.5f; // 50% fragmentation threshold
        bool m_poolingEnabled = true;
        
        // Internal methods
        MemoryChunk* FindFreeChunk(size_t size);
        MemoryChunk* AllocateNewChunk(size_t size);
        void SplitChunk(MemoryChunk* chunk, size_t size);
        void MergeAdjacentChunks();
        void DefragmentPool();
        bool ShouldDefragment() const;
    };

    // RAII wrapper for pool-allocated memory
    template<typename T>
    class PoolAllocatedResource {
    public:
        PoolAllocatedResource(ResourceMemoryPool& pool) : m_pool(pool) {
            m_data = static_cast<T*>(m_pool.Allocate(sizeof(T), alignof(T)));
            if (m_data) {
                new(m_data) T();
            }
        }
        
        template<typename... Args>
        PoolAllocatedResource(ResourceMemoryPool& pool, Args&&... args) : m_pool(pool) {
            m_data = static_cast<T*>(m_pool.Allocate(sizeof(T), alignof(T)));
            if (m_data) {
                new(m_data) T(std::forward<Args>(args)...);
            }
        }
        
        ~PoolAllocatedResource() {
            if (m_data) {
                m_data->~T();
                m_pool.Deallocate(m_data, sizeof(T));
            }
        }
        
        T* get() const { return m_data; }
        T& operator*() const { return *m_data; }
        T* operator->() const { return m_data; }
        
        explicit operator bool() const { return m_data != nullptr; }
        
        // Non-copyable, movable
        PoolAllocatedResource(const PoolAllocatedResource&) = delete;
        PoolAllocatedResource& operator=(const PoolAllocatedResource&) = delete;
        
        PoolAllocatedResource(PoolAllocatedResource&& other) noexcept 
            : m_pool(other.m_pool), m_data(other.m_data) {
            other.m_data = nullptr;
        }
        
        PoolAllocatedResource& operator=(PoolAllocatedResource&& other) noexcept {
            if (this != &other) {
                if (m_data) {
                    m_data->~T();
                    m_pool.Deallocate(m_data, sizeof(T));
                }
                m_data = other.m_data;
                other.m_data = nullptr;
            }
            return *this;
        }
        
    private:
        ResourceMemoryPool& m_pool;
        T* m_data = nullptr;
    };

} // namespace GameEngine