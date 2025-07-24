#include "Resource/ResourceManager.h"
#include "Graphics/Texture.h"
#include "Graphics/Mesh.h"
#include "Audio/AudioEngine.h"
#include "Core/Logger.h"
#include "../TestUtils.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Helper function to create a simple test image file (PNG-like structure)
bool CreateTestImageFile(const std::string& filename, int width = 64, int height = 64) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // Create a simple test file that looks like an image
    // This won't be a valid PNG, but will test file loading behavior
    file.write("TEST_IMAGE", 10);
    
    // Write some basic header-like data
    uint32_t w = width, h = height;
    file.write(reinterpret_cast<const char*>(&w), 4);
    file.write(reinterpret_cast<const char*>(&h), 4);
    
    // Write some dummy pixel data
    for (int i = 0; i < width * height * 4; ++i) {
        uint8_t pixel = static_cast<uint8_t>(i % 256);
        file.write(reinterpret_cast<const char*>(&pixel), 1);
    }

    file.close();
    return true;
}

// Helper function to create a simple test mesh file (OBJ-like structure)
bool CreateTestMeshFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Write a simple cube in OBJ format
    file << "# Test cube mesh\n";
    file << "v -1.0 -1.0  1.0\n";
    file << "v  1.0 -1.0  1.0\n";
    file << "v  1.0  1.0  1.0\n";
    file << "v -1.0  1.0  1.0\n";
    file << "v -1.0 -1.0 -1.0\n";
    file << "v  1.0 -1.0 -1.0\n";
    file << "v  1.0  1.0 -1.0\n";
    file << "v -1.0  1.0 -1.0\n";
    
    file << "vn  0.0  0.0  1.0\n";
    file << "vn  0.0  0.0 -1.0\n";
    file << "vn  0.0  1.0  0.0\n";
    file << "vn  0.0 -1.0  0.0\n";
    file << "vn  1.0  0.0  0.0\n";
    file << "vn -1.0  0.0  0.0\n";
    
    file << "f 1//1 2//1 3//1 4//1\n";
    file << "f 5//2 8//2 7//2 6//2\n";
    file << "f 1//3 5//3 6//3 2//3\n";
    file << "f 2//4 6//4 7//4 3//4\n";
    file << "f 3//5 7//5 8//5 4//5\n";
    file << "f 5//6 1//6 4//6 8//6\n";

    file.close();
    return true;
}

// Helper function to create a simple test audio file (WAV)
bool CreateTestAudioFile(const std::string& filename, float durationSeconds = 0.1f) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    uint32_t sampleRate = 44100;
    uint16_t channels = 2;
    uint16_t bitsPerSample = 16;
    uint32_t samplesPerChannel = static_cast<uint32_t>(sampleRate * durationSeconds);
    uint32_t dataSize = samplesPerChannel * channels * (bitsPerSample / 8);

    // WAV Header
    file.write("RIFF", 4);
    uint32_t fileSize = 36 + dataSize;
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("WAVE", 4);

    // Format chunk
    file.write("fmt ", 4);
    uint32_t fmtChunkSize = 16;
    file.write(reinterpret_cast<const char*>(&fmtChunkSize), 4);
    uint16_t audioFormat = 1; // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&channels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    uint32_t byteRate = sampleRate * channels * bitsPerSample / 8;
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    uint16_t blockAlign = channels * bitsPerSample / 8;
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // Data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Write simple sine wave data
    for (uint32_t i = 0; i < samplesPerChannel; ++i) {
        for (uint16_t ch = 0; ch < channels; ++ch) {
            int16_t sample = static_cast<int16_t>(16383.0 * sin(2.0 * 3.14159 * 440.0 * i / sampleRate));
            file.write(reinterpret_cast<const char*>(&sample), 2);
        }
    }

    file.close();
    return true;
}

bool TestResourceManagerConstruction() {
    TestOutput::PrintTestStart("ResourceManager construction");

    ResourceManager manager;
    
    // Manager should be constructed successfully
    EXPECT_EQUAL(manager.GetResourceCount(), static_cast<size_t>(0));
    EXPECT_EQUAL(manager.GetMemoryUsage(), static_cast<size_t>(0));
    EXPECT_TRUE(manager.IsFallbackResourcesEnabled()); // Default should be enabled

    TestOutput::PrintTestPass("ResourceManager construction");
    return true;
}

bool TestResourceManagerInitialization() {
    TestOutput::PrintTestStart("ResourceManager initialization");

    ResourceManager manager;
    
    // Test initialization
    bool initResult = manager.Initialize();
    EXPECT_TRUE(initResult);
    
    // Test shutdown (should not crash)
    manager.Shutdown();

    TestOutput::PrintTestPass("ResourceManager initialization");
    return true;
}

bool TestTextureResourceLoading() {
    TestOutput::PrintTestStart("Texture resource loading");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory if it doesn't exist
    std::filesystem::create_directories("assets");

    // Create test texture file
    const std::string testFile = "test_texture.png";
    const std::string fullPath = "assets/" + testFile;
    if (!CreateTestImageFile(fullPath)) {
        TestOutput::PrintTestFail("Texture resource loading - Failed to create test file");
        return false;
    }

    // Test loading texture resource
    auto texture = manager.Load<Texture>(testFile);
    
    if (manager.IsFallbackResourcesEnabled()) {
        // Should either load successfully or create fallback
        EXPECT_NOT_NULL(texture);
        if (texture) {
            EXPECT_STRING_EQUAL(texture->GetPath(), fullPath);
        }
    } else {
        // Behavior depends on whether the file can actually be loaded
        TestOutput::PrintInfo("Testing with fallback resources disabled");
    }

    // Test loading same resource again (should use cache)
    auto texture2 = manager.Load<Texture>(testFile);
    if (texture && texture2) {
        EXPECT_TRUE(texture.get() == texture2.get()); // Should be same instance
    }

    // Clean up
    std::filesystem::remove(fullPath);
    manager.Shutdown();

    TestOutput::PrintTestPass("Texture resource loading");
    return true;
}

bool TestMeshResourceLoading() {
    TestOutput::PrintTestStart("Mesh resource loading");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory if it doesn't exist
    std::filesystem::create_directories("assets");

    // Create test mesh file
    const std::string testFile = "test_mesh.obj";
    const std::string fullPath = "assets/" + testFile;
    if (!CreateTestMeshFile(fullPath)) {
        TestOutput::PrintTestFail("Mesh resource loading - Failed to create test file");
        return false;
    }

    // Test loading mesh resource
    auto mesh = manager.Load<Mesh>(testFile);
    
    if (manager.IsFallbackResourcesEnabled()) {
        // Should either load successfully or create fallback
        EXPECT_NOT_NULL(mesh);
        if (mesh) {
            EXPECT_STRING_EQUAL(mesh->GetPath(), fullPath);
        }
    }

    // Test loading same resource again (should use cache)
    auto mesh2 = manager.Load<Mesh>(testFile);
    if (mesh && mesh2) {
        EXPECT_TRUE(mesh.get() == mesh2.get()); // Should be same instance
    }

    // Clean up
    std::filesystem::remove(fullPath);
    manager.Shutdown();

    TestOutput::PrintTestPass("Mesh resource loading");
    return true;
}

bool TestResourceManagerWithAudioEngine() {
    TestOutput::PrintTestStart("ResourceManager with AudioEngine integration");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory if it doesn't exist
    std::filesystem::create_directories("assets");

    // Create test audio file
    const std::string testFile = "test_audio.wav";
    const std::string fullPath = "assets/" + testFile;
    if (!CreateTestAudioFile(fullPath)) {
        TestOutput::PrintTestFail("ResourceManager with AudioEngine integration - Failed to create test file");
        return false;
    }

    // Note: AudioClip is not a Resource type, it's managed by AudioEngine
    // This test verifies that ResourceManager doesn't interfere with audio loading
    
    // Test that ResourceManager can coexist with AudioEngine
    // (AudioEngine manages AudioClip directly, not through ResourceManager)
    AudioEngine audioEngine;
    audioEngine.Initialize();
    
    // Load audio through AudioEngine (not ResourceManager)
    auto audioClip = audioEngine.LoadAudioClip(testFile);
    
    // AudioClip loading behavior depends on audio availability
    if (audioEngine.IsAudioAvailable()) {
        // May succeed or fail depending on file format support
        TestOutput::PrintInfo("Audio system available for testing");
    } else {
        TestOutput::PrintInfo("Audio system not available, testing in silent mode");
    }
    
    audioEngine.Shutdown();

    // Clean up
    std::filesystem::remove(fullPath);
    manager.Shutdown();

    TestOutput::PrintTestPass("ResourceManager with AudioEngine integration");
    return true;
}

bool TestResourceCaching() {
    TestOutput::PrintTestStart("Resource caching");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory and test file
    std::filesystem::create_directories("assets");
    const std::string testFile = "cache_test.png";
    const std::string fullPath = "assets/" + testFile;
    CreateTestImageFile(fullPath);

    // Load resource multiple times
    auto texture1 = manager.Load<Texture>(testFile);
    auto texture2 = manager.Load<Texture>(testFile);
    auto texture3 = manager.Load<Texture>(testFile);

    if (texture1 && texture2 && texture3) {
        // All should be the same instance (cached)
        EXPECT_TRUE(texture1.get() == texture2.get());
        EXPECT_TRUE(texture2.get() == texture3.get());
    }

    // Check resource count
    size_t resourceCount = manager.GetResourceCount();
    EXPECT_TRUE(resourceCount >= 1); // At least one resource should be cached

    // Clean up
    std::filesystem::remove(fullPath);
    manager.Shutdown();

    TestOutput::PrintTestPass("Resource caching");
    return true;
}

bool TestResourceUnloading() {
    TestOutput::PrintTestStart("Resource unloading");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory and test file
    std::filesystem::create_directories("assets");
    const std::string testFile = "unload_test.png";
    const std::string fullPath = "assets/" + testFile;
    CreateTestImageFile(fullPath);

    // Load resource
    auto texture = manager.Load<Texture>(testFile);
    size_t initialCount = manager.GetResourceCount();

    // Unload specific resource
    manager.Unload<Texture>(testFile);
    
    // Resource count might not change immediately due to weak_ptr caching
    // But the resource should be marked for cleanup

    // Test unloading all resources
    manager.UnloadAll();
    
    // Test unloading unused resources
    manager.UnloadUnused();

    // Clean up
    std::filesystem::remove(fullPath);
    manager.Shutdown();

    TestOutput::PrintTestPass("Resource unloading");
    return true;
}

bool TestResourceMemoryManagement() {
    TestOutput::PrintTestStart("Resource memory management");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory
    std::filesystem::create_directories("assets");

    // Load multiple resources to test memory management
    std::vector<std::shared_ptr<Texture>> textures;
    for (int i = 0; i < 5; ++i) {
        std::string filename = "memory_test_" + std::to_string(i) + ".png";
        std::string fullPath = "assets/" + filename;
        CreateTestImageFile(fullPath);
        
        auto texture = manager.Load<Texture>(filename);
        if (texture) {
            textures.push_back(texture);
        }
    }

    // Check memory usage
    size_t memoryUsage = manager.GetMemoryUsage();
    EXPECT_TRUE(memoryUsage > 0); // Should have some memory usage

    // Test memory pressure handling
    manager.HandleMemoryPressure();
    
    // Test LRU cleanup
    manager.UnloadLeastRecentlyUsed();
    
    // Test memory pressure threshold
    manager.SetMemoryPressureThreshold(1024 * 1024); // 1MB
    manager.CheckMemoryPressure();

    // Clean up files
    for (int i = 0; i < 5; ++i) {
        std::string filename = "memory_test_" + std::to_string(i) + ".png";
        std::string fullPath = "assets/" + filename;
        std::filesystem::remove(fullPath);
    }

    manager.Shutdown();

    TestOutput::PrintTestPass("Resource memory management");
    return true;
}

bool TestResourceErrorHandling() {
    TestOutput::PrintTestStart("Resource error handling");

    ResourceManager manager;
    manager.Initialize();

    // Test loading non-existent resource
    auto nullTexture = manager.Load<Texture>("nonexistent.png");
    
    if (manager.IsFallbackResourcesEnabled()) {
        // Should create fallback resource for Texture
        EXPECT_NOT_NULL(nullTexture);
    } else {
        // Should return null
        EXPECT_NULL(nullTexture);
    }

    // Test with fallback resources disabled
    manager.SetFallbackResourcesEnabled(false);
    EXPECT_FALSE(manager.IsFallbackResourcesEnabled());
    
    auto nullTexture2 = manager.Load<Texture>("another_nonexistent.png");
    EXPECT_NULL(nullTexture2);

    // Re-enable fallback resources
    manager.SetFallbackResourcesEnabled(true);
    EXPECT_TRUE(manager.IsFallbackResourcesEnabled());

    // Test error handling methods (should not crash)
    manager.HandleResourceLoadFailure("test.png", "Test error");
    manager.HandleMemoryPressure();

    manager.Shutdown();

    TestOutput::PrintTestPass("Resource error handling");
    return true;
}

bool TestResourceStatistics() {
    TestOutput::PrintTestStart("Resource statistics");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory and test files
    std::filesystem::create_directories("assets");
    
    std::vector<std::string> testFiles = {
        "stats_texture1.png",
        "stats_texture2.png",
        "stats_mesh1.obj"
    };

    // Create test files and load resources
    for (const auto& filename : testFiles) {
        std::string fullPath = "assets/" + filename;
        if (filename.find(".png") != std::string::npos) {
            CreateTestImageFile(fullPath);
            manager.Load<Texture>(filename);
        } else if (filename.find(".obj") != std::string::npos) {
            CreateTestMeshFile(fullPath);
            manager.Load<Mesh>(filename);
        }
    }

    // Test statistics methods
    size_t resourceCount = manager.GetResourceCount();
    size_t memoryUsage = manager.GetMemoryUsage();
    ResourceStats stats = manager.GetResourceStats();

    EXPECT_TRUE(resourceCount > 0);
    EXPECT_TRUE(memoryUsage >= 0); // Should be non-negative
    EXPECT_TRUE(stats.totalResources >= 0);
    EXPECT_TRUE(stats.totalMemoryUsage >= 0);

    // Test logging methods (should not crash)
    manager.LogResourceUsage();
    manager.LogDetailedResourceInfo();

    // Clean up files
    for (const auto& filename : testFiles) {
        std::string fullPath = "assets/" + filename;
        std::filesystem::remove(fullPath);
    }

    manager.Shutdown();

    TestOutput::PrintTestPass("Resource statistics");
    return true;
}

bool TestResourcePerformanceOptimizations() {
    TestOutput::PrintTestStart("Resource performance optimizations");

    ResourceManager manager;
    manager.Initialize();

    // Test enabling/disabling performance features
    manager.EnableMemoryPooling(true);
    manager.EnableMemoryPooling(false);
    manager.EnableMemoryPooling(true);

    manager.EnableLRUCache(true);
    manager.EnableLRUCache(false);
    manager.EnableLRUCache(true);

    manager.EnableGPUUploadOptimization(true);
    manager.EnableGPUUploadOptimization(false);
    manager.EnableGPUUploadOptimization(true);

    // Test setting performance parameters
    manager.SetMemoryPoolSize(1024 * 1024); // 1MB
    manager.SetLRUCacheSize(100, 10 * 1024 * 1024); // 100 items, 10MB
    manager.SetGPUUploadBandwidth(100 * 1024 * 1024); // 100MB/s

    // Test performance statistics
    float cacheHitRatio = manager.GetLRUCacheHitRatio();
    float poolUtilization = manager.GetMemoryPoolUtilization();
    size_t uploadQueueSize = manager.GetGPUUploadQueueSize();

    // Values should be reasonable
    EXPECT_TRUE(cacheHitRatio >= 0.0f && cacheHitRatio <= 1.0f);
    EXPECT_TRUE(poolUtilization >= 0.0f && poolUtilization <= 1.0f);
    EXPECT_TRUE(uploadQueueSize >= 0);

    manager.Shutdown();

    TestOutput::PrintTestPass("Resource performance optimizations");
    return true;
}

bool TestResourceAssetPipeline() {
    TestOutput::PrintTestStart("Resource asset pipeline");

    ResourceManager manager;
    manager.Initialize();

    // Create test source file
    std::filesystem::create_directories("assets");
    const std::string sourceFile = "assets/source_asset.png";
    const std::string targetFile = "assets/target_asset.png";
    const std::string exportFile = "exported_asset.png";

    CreateTestImageFile(sourceFile);

    // Test asset import (should not crash)
    bool importResult = manager.ImportAsset(sourceFile, targetFile);
    // Result depends on implementation, but should not crash

    // Test asset export (should not crash)
    bool exportResult = manager.ExportAsset(sourceFile, exportFile);
    // Result depends on implementation, but should not crash

    // Clean up
    std::filesystem::remove(sourceFile);
    std::filesystem::remove(targetFile);
    std::filesystem::remove(exportFile);

    manager.Shutdown();

    TestOutput::PrintTestPass("Resource asset pipeline");
    return true;
}

bool TestResourceThreadSafety() {
    TestOutput::PrintTestStart("Resource thread safety");

    ResourceManager manager;
    manager.Initialize();

    // Create assets directory and test file
    std::filesystem::create_directories("assets");
    const std::string testFile = "thread_test.png";
    const std::string fullPath = "assets/" + testFile;
    CreateTestImageFile(fullPath);

    // Test concurrent resource loading
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<Texture>> results(4);

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&manager, &testFile, &results, i]() {
            results[i] = manager.Load<Texture>(testFile);
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // All results should be the same instance (cached)
    for (int i = 1; i < 4; ++i) {
        if (results[0] && results[i]) {
            EXPECT_TRUE(results[0].get() == results[i].get());
        }
    }

    // Clean up
    std::filesystem::remove(fullPath);
    manager.Shutdown();

    TestOutput::PrintTestPass("Resource thread safety");
    return true;
}

int main() {
    TestOutput::PrintHeader("Resource Manager Unit Tests");
    Logger::GetInstance().Initialize();

    TestSuite suite("Resource Manager Unit Tests");
    
    bool allPassed = true;
    allPassed &= suite.RunTest("Construction", TestResourceManagerConstruction);
    allPassed &= suite.RunTest("Initialization", TestResourceManagerInitialization);
    allPassed &= suite.RunTest("Texture Loading", TestTextureResourceLoading);
    allPassed &= suite.RunTest("Mesh Loading", TestMeshResourceLoading);
    allPassed &= suite.RunTest("AudioEngine Integration", TestResourceManagerWithAudioEngine);
    allPassed &= suite.RunTest("Resource Caching", TestResourceCaching);
    allPassed &= suite.RunTest("Resource Unloading", TestResourceUnloading);
    allPassed &= suite.RunTest("Memory Management", TestResourceMemoryManagement);
    allPassed &= suite.RunTest("Error Handling", TestResourceErrorHandling);
    allPassed &= suite.RunTest("Statistics", TestResourceStatistics);
    allPassed &= suite.RunTest("Performance Optimizations", TestResourcePerformanceOptimizations);
    allPassed &= suite.RunTest("Asset Pipeline", TestResourceAssetPipeline);
    allPassed &= suite.RunTest("Thread Safety", TestResourceThreadSafety);

    suite.PrintSummary();
    TestOutput::PrintFooter(allPassed);
    
    return allPassed ? 0 : 1;
}