#include "Resource/GPUUploadOptimizer.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Core/Logger.h"
#include <algorithm>
#include <cstring>

namespace GameEngine {

    GPUUploadOptimizer::GPUUploadOptimizer() 
        : m_uploadQueue([](const GPUUploadTask& a, const GPUUploadTask& b) {
            // Higher priority first, then by submit time (FIFO for same priority)
            if (a.priority != b.priority) {
                return a.priority < b.priority;
            }
            return a.submitTime > b.submitTime;
        }) {
        LOG_DEBUG("GPUUploadOptimizer created");
    }

    GPUUploadOptimizer::~GPUUploadOptimizer() {
        Shutdown();
    }

    bool GPUUploadOptimizer::Initialize() {
        LOG_INFO("Initializing GPUUploadOptimizer...");
        
        try {
            // Preallocate upload buffers
            PreallocateUploadBuffers();
            
            // Start async upload thread if enabled
            if (m_asyncUploadsEnabled) {
                m_uploadThread = std::thread(&GPUUploadOptimizer::UploadThreadFunction, this);
                LOG_INFO("GPUUploadOptimizer async upload thread started");
            }
            
            LOG_INFO("GPUUploadOptimizer initialized successfully");
            LOG_INFO("  Max upload bandwidth: " + std::to_string(m_maxUploadBandwidth / 1024 / 1024) + " MB/s");
            LOG_INFO("  Max upload time per frame: " + std::to_string(m_maxUploadTime.count()) + " ms");
            LOG_INFO("  Upload buffer size: " + std::to_string(m_uploadBufferSize / 1024 / 1024) + " MB");
            LOG_INFO("  Async uploads: " + std::string(m_asyncUploadsEnabled ? "enabled" : "disabled"));
            
            return true;
            
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to initialize GPUUploadOptimizer: " + std::string(e.what()));
            return false;
        }
    }

    void GPUUploadOptimizer::Shutdown() {
        LOG_INFO("Shutting down GPUUploadOptimizer...");
        
        // Signal shutdown to async thread
        m_shutdownRequested = true;
        m_uploadCondition.notify_all();
        
        // Wait for thread to finish
        if (m_uploadThread.joinable()) {
            m_uploadThread.join();
            LOG_DEBUG("GPUUploadOptimizer async thread stopped");
        }
        
        // Process any remaining uploads synchronously
        FlushPendingUploads();
        
        // Cleanup upload buffers
        CleanupUploadBuffers();
        
        LOG_INFO("GPUUploadOptimizer shutdown complete");
        LOG_INFO("  Total uploads processed: " + std::to_string(m_totalUploads.load()));
        LOG_INFO("  Total data uploaded: " + std::to_string(m_totalUploadSize.load() / 1024 / 1024) + " MB");
        
        if (m_totalUploads > 0) {
            auto avgTime = m_totalUploadDuration.count() / m_totalUploads.load();
            LOG_INFO("  Average upload time: " + std::to_string(avgTime) + " ms");
        }
    }

    void GPUUploadOptimizer::ScheduleTextureUpload(std::shared_ptr<Texture> texture, int priority) {
        if (!texture) {
            LOG_WARNING("GPUUploadOptimizer: Attempted to schedule null texture upload");
            return;
        }
        
        auto uploadFunc = [texture]() {
            // This would call the actual texture upload method
            // texture->UploadToGPU();
            LOG_DEBUG("Uploading texture: " + texture->GetPath());
        };
        
        size_t dataSize = texture->GetMemoryUsage();
        
        std::lock_guard<std::mutex> lock(m_taskMutex);
        m_uploadQueue.emplace(GPUUploadTask::Type::Texture, texture, uploadFunc, dataSize, priority);
        
        LOG_DEBUG("GPUUploadOptimizer: Scheduled texture upload (" + std::to_string(dataSize / 1024) + " KB, priority " + std::to_string(priority) + ")");
        
        if (m_asyncUploadsEnabled) {
            m_uploadCondition.notify_one();
        }
    }

    void GPUUploadOptimizer::ScheduleMeshUpload(std::shared_ptr<Mesh> mesh, int priority) {
        if (!mesh) {
            LOG_WARNING("GPUUploadOptimizer: Attempted to schedule null mesh upload");
            return;
        }
        
        auto uploadFunc = [mesh]() {
            // This would call the actual mesh upload method
            // mesh->UploadToGPU();
            LOG_DEBUG("Uploading mesh: " + mesh->GetPath());
        };
        
        size_t dataSize = mesh->GetMemoryUsage();
        
        std::lock_guard<std::mutex> lock(m_taskMutex);
        m_uploadQueue.emplace(GPUUploadTask::Type::Mesh, mesh, uploadFunc, dataSize, priority);
        
        LOG_DEBUG("GPUUploadOptimizer: Scheduled mesh upload (" + std::to_string(dataSize / 1024) + " KB, priority " + std::to_string(priority) + ")");
        
        if (m_asyncUploadsEnabled) {
            m_uploadCondition.notify_one();
        }
    }

    void GPUUploadOptimizer::ScheduleCustomUpload(std::function<void()> uploadFunc, size_t dataSize, int priority) {
        if (!uploadFunc) {
            LOG_WARNING("GPUUploadOptimizer: Attempted to schedule null upload function");
            return;
        }
        
        std::lock_guard<std::mutex> lock(m_taskMutex);
        m_uploadQueue.emplace(GPUUploadTask::Type::Buffer, nullptr, uploadFunc, dataSize, priority);
        
        LOG_DEBUG("GPUUploadOptimizer: Scheduled custom upload (" + std::to_string(dataSize / 1024) + " KB, priority " + std::to_string(priority) + ")");
        
        if (m_asyncUploadsEnabled) {
            m_uploadCondition.notify_one();
        }
    }

    void GPUUploadOptimizer::ProcessUploads(size_t maxUploadsPerFrame) {
        if (m_asyncUploadsEnabled) {
            // Async mode - uploads are processed in background thread
            return;
        }
        
        auto startTime = std::chrono::steady_clock::now();
        size_t uploadsProcessed = 0;
        
        std::unique_lock<std::mutex> lock(m_taskMutex);
        
        while (!m_uploadQueue.empty() && uploadsProcessed < maxUploadsPerFrame) {
            // Check time limit
            if (IsWithinTimeLimit(startTime)) {
                break;
            }
            
            GPUUploadTask task = m_uploadQueue.top();
            m_uploadQueue.pop();
            
            // Check bandwidth limit
            if (!IsWithinBandwidthLimit(task.dataSize)) {
                // Put task back and break
                m_uploadQueue.push(task);
                break;
            }
            
            lock.unlock();
            
            // Process the upload
            auto uploadStart = std::chrono::steady_clock::now();
            ProcessUploadTask(task);
            auto uploadEnd = std::chrono::steady_clock::now();
            
            auto uploadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(uploadEnd - uploadStart);
            UpdateBandwidthStats(task.dataSize, uploadDuration);
            
            uploadsProcessed++;
            
            lock.lock();
        }
        
        if (uploadsProcessed > 0) {
            LOG_DEBUG("GPUUploadOptimizer: Processed " + std::to_string(uploadsProcessed) + " uploads this frame");
        }
    }

    void GPUUploadOptimizer::ProcessAllUploads() {
        LOG_INFO("GPUUploadOptimizer: Processing all pending uploads");
        
        std::unique_lock<std::mutex> lock(m_taskMutex);
        size_t totalUploads = m_uploadQueue.size();
        
        while (!m_uploadQueue.empty()) {
            GPUUploadTask task = m_uploadQueue.top();
            m_uploadQueue.pop();
            
            lock.unlock();
            ProcessUploadTask(task);
            lock.lock();
        }
        
        LOG_INFO("GPUUploadOptimizer: Processed " + std::to_string(totalUploads) + " uploads");
    }

    void GPUUploadOptimizer::FlushPendingUploads() {
        ProcessAllUploads();
    }

    void GPUUploadOptimizer::SetMaxUploadBandwidth(size_t bytesPerSecond) {
        m_maxUploadBandwidth = bytesPerSecond;
        LOG_INFO("GPUUploadOptimizer: Max upload bandwidth set to " + std::to_string(bytesPerSecond / 1024 / 1024) + " MB/s");
    }

    void GPUUploadOptimizer::SetMaxUploadTime(std::chrono::milliseconds maxTime) {
        m_maxUploadTime = maxTime;
        LOG_INFO("GPUUploadOptimizer: Max upload time per frame set to " + std::to_string(maxTime.count()) + " ms");
    }

    void GPUUploadOptimizer::EnableAsyncUploads(bool enabled) {
        if (enabled != m_asyncUploadsEnabled) {
            LOG_INFO("GPUUploadOptimizer: Async uploads " + std::string(enabled ? "enabled" : "disabled"));
            
            if (enabled && !m_uploadThread.joinable()) {
                m_asyncUploadsEnabled = true;
                m_uploadThread = std::thread(&GPUUploadOptimizer::UploadThreadFunction, this);
            } else if (!enabled && m_uploadThread.joinable()) {
                m_shutdownRequested = true;
                m_uploadCondition.notify_all();
                m_uploadThread.join();
                m_shutdownRequested = false;
                m_asyncUploadsEnabled = false;
            }
        }
    }

    size_t GPUUploadOptimizer::GetPendingUploadCount() const {
        std::lock_guard<std::mutex> lock(m_taskMutex);
        return m_uploadQueue.size();
    }

    size_t GPUUploadOptimizer::GetPendingUploadSize() const {
        std::lock_guard<std::mutex> lock(m_taskMutex);
        
        size_t totalSize = 0;
        auto queueCopy = m_uploadQueue; // Copy to avoid modifying original
        
        while (!queueCopy.empty()) {
            totalSize += queueCopy.top().dataSize;
            queueCopy.pop();
        }
        
        return totalSize;
    }

    float GPUUploadOptimizer::GetUploadBandwidthUsage() const {
        if (m_maxUploadBandwidth == 0) {
            return 0.0f;
        }
        
        // Calculate bandwidth usage over the last second
        auto now = std::chrono::steady_clock::now();
        auto timeSinceLastUpload = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastUploadTime);
        
        if (timeSinceLastUpload.count() > 1) {
            return 0.0f; // No recent uploads
        }
        
        // This is a simplified calculation - in practice you'd want to track bandwidth over a rolling window
        return std::min(1.0f, static_cast<float>(m_totalUploadSize.load()) / m_maxUploadBandwidth);
    }

    std::chrono::milliseconds GPUUploadOptimizer::GetAverageUploadTime() const {
        size_t totalUploads = m_totalUploads.load();
        if (totalUploads == 0) {
            return std::chrono::milliseconds(0);
        }
        
        return std::chrono::milliseconds(m_totalUploadDuration.count() / totalUploads);
    }

    void GPUUploadOptimizer::SetUploadBufferSize(size_t size) {
        m_uploadBufferSize = size;
        LOG_INFO("GPUUploadOptimizer: Upload buffer size set to " + std::to_string(size / 1024 / 1024) + " MB");
        
        // Recreate buffers with new size
        CleanupUploadBuffers();
        PreallocateUploadBuffers();
    }

    void GPUUploadOptimizer::PreallocateUploadBuffers() {
        LOG_DEBUG("GPUUploadOptimizer: Preallocating upload buffers");
        
#ifdef GAMEENGINE_HAS_OPENGL
        try {
            // Create a few upload buffers for different sizes
            std::vector<size_t> bufferSizes = {
                m_uploadBufferSize / 4,  // Small uploads
                m_uploadBufferSize / 2,  // Medium uploads
                m_uploadBufferSize       // Large uploads
            };
            
            for (size_t size : bufferSizes) {
                UploadBuffer buffer;
                buffer.size = size;
                
                glGenBuffers(1, &buffer.bufferId);
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer.bufferId);
                glBufferData(GL_PIXEL_UNPACK_BUFFER, size, nullptr, GL_STREAM_DRAW);
                
                // Map buffer for persistent mapping if supported
                buffer.mappedPtr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
                
                if (buffer.mappedPtr) {
                    m_uploadBuffers.push_back(buffer);
                    LOG_DEBUG("GPUUploadOptimizer: Created upload buffer (" + std::to_string(size / 1024 / 1024) + " MB)");
                } else {
                    glDeleteBuffers(1, &buffer.bufferId);
                    LOG_WARNING("GPUUploadOptimizer: Failed to map upload buffer");
                }
            }
            
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
            
        } catch (const std::exception& e) {
            LOG_ERROR("GPUUploadOptimizer: Failed to preallocate upload buffers: " + std::string(e.what()));
        }
#endif
    }

    void GPUUploadOptimizer::CleanupUploadBuffers() {
        LOG_DEBUG("GPUUploadOptimizer: Cleaning up upload buffers");
        
#ifdef GAMEENGINE_HAS_OPENGL
        for (auto& buffer : m_uploadBuffers) {
            if (buffer.mappedPtr) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer.bufferId);
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
            }
            
            if (buffer.bufferId != 0) {
                glDeleteBuffers(1, &buffer.bufferId);
            }
        }
        
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#endif
        
        m_uploadBuffers.clear();
    }

    void GPUUploadOptimizer::UploadThreadFunction() {
        LOG_DEBUG("GPUUploadOptimizer: Upload thread started");
        
        while (!m_shutdownRequested) {
            std::unique_lock<std::mutex> lock(m_taskMutex);
            
            // Wait for uploads or shutdown signal
            m_uploadCondition.wait(lock, [this] {
                return !m_uploadQueue.empty() || m_shutdownRequested;
            });
            
            if (m_shutdownRequested) {
                break;
            }
            
            if (!m_uploadQueue.empty()) {
                GPUUploadTask task = m_uploadQueue.top();
                m_uploadQueue.pop();
                
                lock.unlock();
                
                // Process upload in background thread
                auto uploadStart = std::chrono::steady_clock::now();
                ProcessUploadTask(task);
                auto uploadEnd = std::chrono::steady_clock::now();
                
                auto uploadDuration = std::chrono::duration_cast<std::chrono::milliseconds>(uploadEnd - uploadStart);
                UpdateBandwidthStats(task.dataSize, uploadDuration);
            }
        }
        
        LOG_DEBUG("GPUUploadOptimizer: Upload thread stopped");
    }

    void GPUUploadOptimizer::ProcessUploadTask(const GPUUploadTask& task) {
        try {
            auto startTime = std::chrono::steady_clock::now();
            
            // Execute the upload function
            task.uploadFunction();
            
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            m_totalUploads++;
            m_totalUploadSize += task.dataSize;
            m_totalUploadDuration += duration;
            m_lastUploadTime = endTime;
            
            LOG_DEBUG("GPUUploadOptimizer: Processed upload (" + std::to_string(task.dataSize / 1024) + 
                     " KB in " + std::to_string(duration.count()) + " ms)");
            
        } catch (const std::exception& e) {
            LOG_ERROR("GPUUploadOptimizer: Exception during upload: " + std::string(e.what()));
        } catch (...) {
            LOG_ERROR("GPUUploadOptimizer: Unknown exception during upload");
        }
    }

    GPUUploadOptimizer::UploadBuffer* GPUUploadOptimizer::AcquireUploadBuffer(size_t minSize) {
        for (auto& buffer : m_uploadBuffers) {
            if (!buffer.inUse && buffer.size >= minSize) {
                buffer.inUse = true;
                return &buffer;
            }
        }
        
        return nullptr; // No suitable buffer available
    }

    void GPUUploadOptimizer::ReleaseUploadBuffer(UploadBuffer* buffer) {
        if (buffer) {
            buffer->inUse = false;
        }
    }

    void GPUUploadOptimizer::UpdateBandwidthStats(size_t uploadSize, std::chrono::milliseconds duration) {
        // Update bandwidth statistics
        // This is a simplified implementation - in practice you'd want more sophisticated tracking
        
        if (duration.count() > 0) {
            size_t bandwidth = (uploadSize * 1000) / duration.count(); // bytes per second
            
            LOG_DEBUG("GPUUploadOptimizer: Upload bandwidth: " + std::to_string(bandwidth / 1024 / 1024) + " MB/s");
        }
    }

    bool GPUUploadOptimizer::IsWithinBandwidthLimit(size_t uploadSize) const {
        // Simplified bandwidth check - in practice you'd track bandwidth over time
        return uploadSize <= m_maxUploadBandwidth;
    }

    bool GPUUploadOptimizer::IsWithinTimeLimit(std::chrono::steady_clock::time_point startTime) const {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
        return elapsed < m_maxUploadTime;
    }

#ifdef GAMEENGINE_HAS_OPENGL
    void GPUUploadOptimizer::OptimizeTextureFormat(GLenum& internalFormat, GLenum& format, GLenum& type, 
                                                  int width, int height, const void* data) {
        // Texture format optimization implementation
        // This is a stub - would contain actual optimization logic
        LOG_DEBUG("GPUUploadOptimizer: Optimizing texture format for " + std::to_string(width) + "x" + std::to_string(height));
    }

    bool GPUUploadOptimizer::ShouldCompressTexture(int width, int height, GLenum format) const {
        // Simple heuristic - compress large textures
        return (width * height) > (512 * 512);
    }

    void GPUUploadOptimizer::CompressTextureData(const void* input, void*& output, size_t& outputSize,
                                               int width, int height, GLenum format) {
        // Texture compression implementation stub
        LOG_DEBUG("GPUUploadOptimizer: Compressing texture data");
        output = nullptr;
        outputSize = 0;
    }
#endif

    void GPUUploadOptimizer::OptimizeMeshData(std::vector<float>& vertices, std::vector<unsigned int>& indices) {
        // Mesh optimization implementation stub
        LOG_DEBUG("GPUUploadOptimizer: Optimizing mesh data (" + std::to_string(vertices.size()) + " vertices, " + 
                 std::to_string(indices.size()) + " indices)");
    }

    void GPUUploadOptimizer::OptimizeVertexLayout(std::vector<float>& vertices, size_t vertexSize) {
        // Vertex layout optimization stub
        LOG_DEBUG("GPUUploadOptimizer: Optimizing vertex layout");
    }

    void GPUUploadOptimizer::GenerateOptimalIndices(std::vector<unsigned int>& indices) {
        // Index optimization stub
        LOG_DEBUG("GPUUploadOptimizer: Generating optimal indices");
    }

} // namespace GameEngine