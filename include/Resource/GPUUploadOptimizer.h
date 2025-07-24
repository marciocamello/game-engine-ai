#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <chrono>

#ifdef GAMEENGINE_HAS_OPENGL
#include <glad/glad.h>
#endif

namespace GameEngine {

    class Texture;
    class Mesh;

    // GPU upload task for batching and optimization
    struct GPUUploadTask {
        enum class Type {
            Texture,
            Mesh,
            Buffer
        };
        
        Type type;
        std::shared_ptr<void> resource;
        std::function<void()> uploadFunction;
        size_t dataSize = 0;
        int priority = 0; // Higher priority = uploaded first
        std::chrono::steady_clock::time_point submitTime;
        
        GPUUploadTask(Type t, std::shared_ptr<void> res, std::function<void()> func, size_t size, int prio = 0)
            : type(t), resource(res), uploadFunction(func), dataSize(size), priority(prio)
            , submitTime(std::chrono::steady_clock::now()) {}
    };

    // Optimized GPU upload manager for batching and scheduling uploads
    class GPUUploadOptimizer {
    public:
        GPUUploadOptimizer();
        ~GPUUploadOptimizer();

        // Initialization
        bool Initialize();
        void Shutdown();
        
        // Upload scheduling
        void ScheduleTextureUpload(std::shared_ptr<Texture> texture, int priority = 0);
        void ScheduleMeshUpload(std::shared_ptr<Mesh> mesh, int priority = 0);
        void ScheduleCustomUpload(std::function<void()> uploadFunc, size_t dataSize, int priority = 0);
        
        // Batch processing
        void ProcessUploads(size_t maxUploadsPerFrame = 5);
        void ProcessAllUploads();
        void FlushPendingUploads();
        
        // Configuration
        void SetMaxUploadBandwidth(size_t bytesPerSecond);
        void SetMaxUploadTime(std::chrono::milliseconds maxTime);
        void EnableAsyncUploads(bool enabled);
        void SetCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }
        
        // Statistics
        size_t GetPendingUploadCount() const;
        size_t GetPendingUploadSize() const;
        float GetUploadBandwidthUsage() const;
        std::chrono::milliseconds GetAverageUploadTime() const;
        
        // Upload optimization
#ifdef GAMEENGINE_HAS_OPENGL
        void OptimizeTextureFormat(GLenum& internalFormat, GLenum& format, GLenum& type, 
                                  int width, int height, const void* data);
#endif
        void OptimizeMeshData(std::vector<float>& vertices, std::vector<unsigned int>& indices);
        
        // Memory management
        void SetUploadBufferSize(size_t size);
        void PreallocateUploadBuffers();
        void CleanupUploadBuffers();

    private:
        struct UploadBuffer {
#ifdef GAMEENGINE_HAS_OPENGL
            GLuint bufferId = 0;
#else
            unsigned int bufferId = 0;
#endif
            size_t size = 0;
            void* mappedPtr = nullptr;
            bool inUse = false;
        };
        
        mutable std::mutex m_taskMutex;
        std::priority_queue<GPUUploadTask, std::vector<GPUUploadTask>, 
                           std::function<bool(const GPUUploadTask&, const GPUUploadTask&)>> m_uploadQueue;
        
        // Async upload thread
        std::thread m_uploadThread;
        std::condition_variable m_uploadCondition;
        std::atomic<bool> m_shutdownRequested{false};
        bool m_asyncUploadsEnabled = false;
        
        // Upload buffers for optimization
        std::vector<UploadBuffer> m_uploadBuffers;
        size_t m_uploadBufferSize = 16 * 1024 * 1024; // 16MB default
        
        // Performance settings
        size_t m_maxUploadBandwidth = 100 * 1024 * 1024; // 100MB/s default
        std::chrono::milliseconds m_maxUploadTime{16}; // 16ms default (60fps)
        bool m_compressionEnabled = true;
        
        // Statistics
        mutable std::atomic<size_t> m_totalUploads{0};
        mutable std::atomic<size_t> m_totalUploadSize{0};
        mutable std::chrono::steady_clock::time_point m_lastUploadTime;
        mutable std::chrono::milliseconds m_totalUploadDuration{0};
        
        // Internal methods
        void UploadThreadFunction();
        void ProcessUploadTask(const GPUUploadTask& task);
        UploadBuffer* AcquireUploadBuffer(size_t minSize);
        void ReleaseUploadBuffer(UploadBuffer* buffer);
        
        // Optimization helpers
#ifdef GAMEENGINE_HAS_OPENGL
        bool ShouldCompressTexture(int width, int height, GLenum format) const;
        void CompressTextureData(const void* input, void*& output, size_t& outputSize,
                               int width, int height, GLenum format);
#endif
        void OptimizeVertexLayout(std::vector<float>& vertices, size_t vertexSize);
        void GenerateOptimalIndices(std::vector<unsigned int>& indices);
        
        // Performance monitoring
        void UpdateBandwidthStats(size_t uploadSize, std::chrono::milliseconds duration);
        bool IsWithinBandwidthLimit(size_t uploadSize) const;
        bool IsWithinTimeLimit(std::chrono::steady_clock::time_point startTime) const;
    };

} // namespace GameEngine