#include "Resource/ResourceMemoryPool.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <malloc.h>
#endif

namespace GameEngine {

    ResourceMemoryPool::ResourceMemoryPool() {
        LOG_DEBUG("ResourceMemoryPool initialized with " + std::to_string(m_poolSize / 1024 / 1024) + " MB pool size");
    }

    ResourceMemoryPool::~ResourceMemoryPool() {
        Clear();
        LOG_DEBUG("ResourceMemoryPool destroyed");
    }

    void* ResourceMemoryPool::Allocate(size_t size, size_t alignment) {
        if (!m_poolingEnabled) {
            // Fallback to standard allocation
#ifdef _WIN32
            return _aligned_malloc(size, alignment);
#else
            return std::aligned_alloc(alignment, size);
#endif
        }
        
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        // Align size to the requested alignment
        size_t alignedSize = (size + alignment - 1) & ~(alignment - 1);
        
        // Try to find a free chunk
        MemoryChunk* chunk = FindFreeChunk(alignedSize);
        
        if (!chunk) {
            // Allocate new chunk
            chunk = AllocateNewChunk(alignedSize);
            if (!chunk) {
                LOG_ERROR("ResourceMemoryPool failed to allocate " + std::to_string(size) + " bytes");
                return nullptr;
            }
        }
        
        // Mark chunk as in use
        chunk->inUse = true;
        m_allocatedChunks[chunk->data] = chunk;
        m_totalAllocated += chunk->size;
        
        LOG_DEBUG("ResourceMemoryPool allocated " + std::to_string(size) + " bytes (pool utilization: " + 
                 std::to_string(GetUtilization() * 100.0f) + "%)");
        
        return chunk->data;
    }

    void ResourceMemoryPool::Deallocate(void* ptr, size_t size) {
        if (!ptr) {
            return;
        }
        
        if (!m_poolingEnabled) {
            // Fallback to standard deallocation
#ifdef _WIN32
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        auto it = m_allocatedChunks.find(ptr);
        if (it != m_allocatedChunks.end()) {
            MemoryChunk* chunk = it->second;
            chunk->inUse = false;
            m_totalAllocated -= chunk->size;
            m_allocatedChunks.erase(it);
            
            LOG_DEBUG("ResourceMemoryPool deallocated " + std::to_string(size) + " bytes");
            
            // Check if we should defragment
            if (ShouldDefragment()) {
                DefragmentPool();
            }
        } else {
            LOG_WARNING("ResourceMemoryPool attempted to deallocate unknown pointer");
        }
    }

    void ResourceMemoryPool::SetPoolSize(size_t poolSize) {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        if (poolSize != m_poolSize) {
            LOG_INFO("ResourceMemoryPool changing pool size from " + 
                    std::to_string(m_poolSize / 1024 / 1024) + " MB to " + 
                    std::to_string(poolSize / 1024 / 1024) + " MB");
            
            m_poolSize = poolSize;
            
            // If we have allocated memory, we might need to adjust
            if (!m_memoryBlocks.empty()) {
                LOG_INFO("ResourceMemoryPool will apply new size on next preallocation");
            }
        }
    }

    void ResourceMemoryPool::SetChunkSize(size_t chunkSize) {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        m_chunkSize = chunkSize;
        LOG_DEBUG("ResourceMemoryPool chunk size set to " + std::to_string(chunkSize) + " bytes");
    }

    void ResourceMemoryPool::PreallocatePool() {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        LOG_INFO("ResourceMemoryPool preallocating " + std::to_string(m_poolSize / 1024 / 1024) + " MB");
        
        try {
            MemoryBlock block;
            block.size = m_poolSize;
            block.data = 
#ifdef _WIN32
                _aligned_malloc(m_poolSize, alignof(std::max_align_t));
#else
                std::aligned_alloc(alignof(std::max_align_t), m_poolSize);
#endif
            
            if (!block.data) {
                LOG_ERROR("ResourceMemoryPool failed to preallocate memory pool");
                return;
            }
            
            // Initialize chunks
            size_t numChunks = m_poolSize / m_chunkSize;
            block.chunks.resize(numChunks);
            
            char* chunkPtr = static_cast<char*>(block.data);
            for (size_t i = 0; i < numChunks; ++i) {
                block.chunks[i].data = chunkPtr + (i * m_chunkSize);
                block.chunks[i].size = m_chunkSize;
                block.chunks[i].inUse = false;
                
                // Link chunks together
                if (i < numChunks - 1) {
                    block.chunks[i].next = &block.chunks[i + 1];
                }
            }
            
            m_memoryBlocks.push_back(std::move(block));
            m_totalPoolSize += m_poolSize;
            
            LOG_INFO("ResourceMemoryPool preallocation complete (" + std::to_string(numChunks) + " chunks)");
            
        } catch (const std::exception& e) {
            LOG_ERROR("ResourceMemoryPool preallocation failed: " + std::string(e.what()));
        }
    }

    void ResourceMemoryPool::ShrinkToFit() {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        LOG_INFO("ResourceMemoryPool shrinking to fit");
        
        // Remove empty blocks
        size_t removedBlocks = 0;
        size_t freedMemory = 0;
        
        for (auto it = m_memoryBlocks.begin(); it != m_memoryBlocks.end();) {
            bool blockInUse = false;
            
            // Check if any chunks in this block are in use
            for (const auto& chunk : it->chunks) {
                if (chunk.inUse) {
                    blockInUse = true;
                    break;
                }
            }
            
            if (!blockInUse) {
                freedMemory += it->size;
#ifdef _WIN32
                _aligned_free(it->data);
#else
                std::free(it->data);
#endif
                it = m_memoryBlocks.erase(it);
                ++removedBlocks;
            } else {
                ++it;
            }
        }
        
        m_totalPoolSize -= freedMemory;
        
        if (removedBlocks > 0) {
            LOG_INFO("ResourceMemoryPool shrink complete: removed " + std::to_string(removedBlocks) + 
                    " blocks, freed " + std::to_string(freedMemory / 1024 / 1024) + " MB");
        }
    }

    void ResourceMemoryPool::Clear() {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        LOG_INFO("ResourceMemoryPool clearing all memory");
        
        // Free all memory blocks
        for (auto& block : m_memoryBlocks) {
            if (block.data) {
#ifdef _WIN32
                _aligned_free(block.data);
#else
                std::free(block.data);
#endif
            }
        }
        
        m_memoryBlocks.clear();
        m_allocatedChunks.clear();
        m_totalAllocated = 0;
        m_totalPoolSize = 0;
    }

    size_t ResourceMemoryPool::GetFragmentation() const {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        if (m_totalPoolSize == 0) {
            return 0;
        }
        
        size_t freeChunks = 0;
        size_t totalChunks = 0;
        
        for (const auto& block : m_memoryBlocks) {
            for (const auto& chunk : block.chunks) {
                ++totalChunks;
                if (!chunk.inUse) {
                    ++freeChunks;
                }
            }
        }
        
        if (totalChunks == 0) {
            return 0;
        }
        
        // Fragmentation is the percentage of free chunks
        return (freeChunks * 100) / totalChunks;
    }

    float ResourceMemoryPool::GetUtilization() const {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        
        if (m_totalPoolSize == 0) {
            return 0.0f;
        }
        
        return static_cast<float>(m_totalAllocated) / m_totalPoolSize;
    }

    ResourceMemoryPool::MemoryChunk* ResourceMemoryPool::FindFreeChunk(size_t size) {
        // Find the best-fit free chunk
        MemoryChunk* bestChunk = nullptr;
        size_t bestSize = SIZE_MAX;
        
        for (auto& block : m_memoryBlocks) {
            for (auto& chunk : block.chunks) {
                if (!chunk.inUse && chunk.size >= size && chunk.size < bestSize) {
                    bestChunk = &chunk;
                    bestSize = chunk.size;
                    
                    // Perfect fit
                    if (chunk.size == size) {
                        break;
                    }
                }
            }
        }
        
        // Split chunk if it's significantly larger than needed
        if (bestChunk && bestChunk->size > size + m_chunkSize) {
            SplitChunk(bestChunk, size);
        }
        
        return bestChunk;
    }

    ResourceMemoryPool::MemoryChunk* ResourceMemoryPool::AllocateNewChunk(size_t size) {
        // Try to allocate from existing blocks first
        for (auto& block : m_memoryBlocks) {
            for (auto& chunk : block.chunks) {
                if (!chunk.inUse && chunk.size >= size) {
                    return &chunk;
                }
            }
        }
        
        // Need to allocate a new block
        size_t blockSize = std::max(m_poolSize, size * 2);
        
        try {
            MemoryBlock block;
            block.size = blockSize;
            block.data = 
#ifdef _WIN32
                _aligned_malloc(blockSize, alignof(std::max_align_t));
#else
                std::aligned_alloc(alignof(std::max_align_t), blockSize);
#endif
            
            if (!block.data) {
                return nullptr;
            }
            
            // Create one large chunk for this block
            block.chunks.resize(1);
            block.chunks[0].data = block.data;
            block.chunks[0].size = blockSize;
            block.chunks[0].inUse = false;
            block.chunks[0].next = nullptr;
            
            m_memoryBlocks.push_back(std::move(block));
            m_totalPoolSize += blockSize;
            
            LOG_DEBUG("ResourceMemoryPool allocated new block: " + std::to_string(blockSize / 1024 / 1024) + " MB");
            
            return &m_memoryBlocks.back().chunks[0];
            
        } catch (const std::exception& e) {
            LOG_ERROR("ResourceMemoryPool failed to allocate new block: " + std::string(e.what()));
            return nullptr;
        }
    }

    void ResourceMemoryPool::SplitChunk(MemoryChunk* chunk, size_t size) {
        if (!chunk || chunk->size <= size) {
            return;
        }
        
        // Find the block containing this chunk
        for (auto& block : m_memoryBlocks) {
            auto it = std::find_if(block.chunks.begin(), block.chunks.end(),
                [chunk](const MemoryChunk& c) { return &c == chunk; });
            
            if (it != block.chunks.end()) {
                // Create new chunk for the remaining space
                MemoryChunk newChunk;
                newChunk.data = static_cast<char*>(chunk->data) + size;
                newChunk.size = chunk->size - size;
                newChunk.inUse = false;
                newChunk.next = chunk->next;
                
                // Update original chunk
                chunk->size = size;
                chunk->next = nullptr;
                
                // Insert new chunk after the original
                block.chunks.insert(it + 1, newChunk);
                break;
            }
        }
    }

    void ResourceMemoryPool::MergeAdjacentChunks() {
        for (auto& block : m_memoryBlocks) {
            for (size_t i = 0; i < block.chunks.size() - 1; ++i) {
                MemoryChunk& current = block.chunks[i];
                MemoryChunk& next = block.chunks[i + 1];
                
                // Check if chunks are adjacent and both free
                if (!current.inUse && !next.inUse) {
                    char* currentEnd = static_cast<char*>(current.data) + current.size;
                    if (currentEnd == next.data) {
                        // Merge chunks
                        current.size += next.size;
                        current.next = next.next;
                        
                        // Remove the next chunk
                        block.chunks.erase(block.chunks.begin() + i + 1);
                        --i; // Check this position again
                    }
                }
            }
        }
    }

    void ResourceMemoryPool::DefragmentPool() {
        LOG_DEBUG("ResourceMemoryPool defragmenting pool");
        
        MergeAdjacentChunks();
        
        // Additional defragmentation logic could be added here
        // such as moving allocated chunks to reduce fragmentation
        
        LOG_DEBUG("ResourceMemoryPool defragmentation complete (fragmentation: " + 
                 std::to_string(GetFragmentation()) + "%)");
    }

    bool ResourceMemoryPool::ShouldDefragment() const {
        float fragmentation = GetFragmentation() / 100.0f;
        return fragmentation > m_defragThreshold;
    }

} // namespace GameEngine