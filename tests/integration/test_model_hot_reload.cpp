/**
 * @file test_model_hot_reload.cpp
 * @brief Integration tests for model hot-reloading functionality
 */

#include "../TestUtils.h"
#include "Resource/ModelHotReloader.h"
#include "Resource/ModelDevelopmentTools.h"
#include "Resource/ModelLoader.h"
#include "Graphics/Model.h"
#include "Core/Logger.h"
#include "Core/Engine.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace GameEngine;
using namespace GameEngine::Testing;

// Test helper to create a temporary model file
bool CreateTestModelFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    file << content;
    file.close();
    return true;
}

// Simple OBJ content for testing
const std::string TEST_OBJ_CONTENT_V1 = R"(
# Test OBJ file v1
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
f 1 2 3
)";

const std::string TEST_OBJ_CONTENT_V2 = R"(
# Test OBJ file v2 - modified
v 0.0 0.0 0.0
v 1.0 0.0 0.0
v 0.0 1.0 0.0
v 1.0 1.0 0.0
f 1 2 3
f 2 3 4
)";

bool TestModelHotReloaderInitialization() {
    TestOutput::PrintTestStart("ModelHotReloader initialization");

    try {
        auto modelLoader = std::make_shared<ModelLoader>();
        if (!modelLoader->Initialize()) {
            TestOutput::PrintTestFail("ModelHotReloader initialization - ModelLoader initialization failed");
            return false;
        }

        ModelHotReloader hotReloader;
        
        // Test initialization
        bool initResult = hotReloader.Initialize(modelLoader);
        if (!initResult) {
            TestOutput::PrintTestFail("ModelHotReloader initialization", "true", "false");
            return false;
        }

        if (!hotReloader.IsInitialized()) {
            TestOutput::PrintTestFail("ModelHotReloader initialization", "initialized", "not initialized");
            return false;
        }

        // Test configuration
        ModelHotReloader::HotReloadConfig config;
        config.enabled = true;
        config.pollInterval = std::chrono::milliseconds(100);
        config.validateOnReload = true;
        hotReloader.SetConfig(config);

        auto retrievedConfig = hotReloader.GetConfig();
        if (retrievedConfig.pollInterval != std::chrono::milliseconds(100)) {
            TestOutput::PrintTestFail("ModelHotReloader initialization", "100ms poll interval", 
                                    std::to_string(retrievedConfig.pollInterval.count()) + "ms");
            return false;
        }

        hotReloader.Shutdown();
        modelLoader->Shutdown();

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelHotReloader initialization", "no exception", e.what());
        return false;
    }

    TestOutput::PrintTestPass("ModelHotReloader initialization");
    return true;
}

bool TestModelWatchingAndReloading() {
    TestOutput::PrintTestStart("model watching and reloading");

    try {
        // Create temporary test file
        std::string testFilePath = "test_model_temp.obj";
        if (!CreateTestModelFile(testFilePath, TEST_OBJ_CONTENT_V1)) {
            TestOutput::PrintTestFail("model watching and reloading", "test file created", "failed to create test file");
            return false;
        }

        TestOutput::PrintInfo("Created test file: " + testFilePath);

        // Initialize engine first (required for ResourceManager)
        auto engine = std::make_unique<Engine>();
        if (!engine->Initialize()) {
            std::filesystem::remove(testFilePath);
            TestOutput::PrintTestFail("model watching and reloading", "engine initialized", "engine initialization failed");
            return false;
        }

        TestOutput::PrintInfo("Engine initialized successfully");

        auto modelLoader = std::make_shared<ModelLoader>();
        if (!modelLoader->Initialize()) {
            std::filesystem::remove(testFilePath);
            engine->Shutdown();
            TestOutput::PrintTestFail("model watching and reloading - ModelLoader initialization failed");
            return false;
        }

        TestOutput::PrintInfo("ModelLoader initialized successfully");

        // Load initial model
        TestOutput::PrintInfo("Loading model: " + testFilePath);
        auto model = modelLoader->LoadModelAsResource(testFilePath);
        if (!model) {
            std::filesystem::remove(testFilePath);
            modelLoader->Shutdown();
            engine->Shutdown();
            TestOutput::PrintTestFail("model watching and reloading", "model loaded", "model is null");
            return false;
        }

        TestOutput::PrintInfo("Model loaded successfully");

        // Initialize hot-reloader
        ModelHotReloader hotReloader;
        if (!hotReloader.Initialize(modelLoader)) {
            std::filesystem::remove(testFilePath);
            modelLoader->Shutdown();
            engine->Shutdown();
            TestOutput::PrintTestFail("model watching and reloading", "hot-reloader initialized", "initialization failed");
            return false;
        }

        // Set up reload callback
        std::atomic<bool> reloadCalled{false};
        std::atomic<bool> reloadSuccess{false};
        hotReloader.SetReloadCallback([&](const std::string& path, std::shared_ptr<Model> newModel, bool success) {
            reloadCalled = true;
            reloadSuccess = success;
        });

        // Configure for fast polling
        ModelHotReloader::HotReloadConfig config;
        config.enabled = true;
        config.pollInterval = std::chrono::milliseconds(50); // Very fast for testing
        config.validateOnReload = false; // Disable validation for simpler test
        hotReloader.SetConfig(config);

        // Start watching the model
        hotReloader.WatchModel(testFilePath, model);
        hotReloader.StartWatching();

        // Wait a bit to ensure watching is active
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Modify the file
        if (!CreateTestModelFile(testFilePath, TEST_OBJ_CONTENT_V2)) {
            std::filesystem::remove(testFilePath);
            hotReloader.Shutdown();
            modelLoader->Shutdown();
            engine->Shutdown();
            TestOutput::PrintTestFail("model watching and reloading", "file modified", "failed to modify file");
            return false;
        }

        // Wait for reload to be detected and processed
        int waitCount = 0;
        const int maxWait = 50; // 5 seconds max wait
        while (!reloadCalled && waitCount < maxWait) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            waitCount++;
        }

        hotReloader.StopWatching();

        if (!reloadCalled) {
            std::filesystem::remove(testFilePath);
            hotReloader.Shutdown();
            modelLoader->Shutdown();
            engine->Shutdown();
            TestOutput::PrintTestFail("model watching and reloading", "reload callback called", "callback not called");
            return false;
        }

        if (!reloadSuccess) {
            std::filesystem::remove(testFilePath);
            hotReloader.Shutdown();
            modelLoader->Shutdown();
            engine->Shutdown();
            TestOutput::PrintTestFail("model watching and reloading", "reload successful", "reload failed");
            return false;
        }

        // Check statistics
        auto stats = hotReloader.GetStats();
        if (stats.totalReloadAttempts == 0) {
            std::filesystem::remove(testFilePath);
            hotReloader.Shutdown();
            modelLoader->Shutdown();
            engine->Shutdown();
            TestOutput::PrintTestFail("model watching and reloading", "reload attempts > 0", "0 reload attempts");
            return false;
        }

        // Cleanup
        std::filesystem::remove(testFilePath);
        hotReloader.Shutdown();
        modelLoader->Shutdown();
        engine->Shutdown();

    } catch (const std::exception& e) {
        std::filesystem::remove("test_model_temp.obj");
        TestOutput::PrintTestFail("model watching and reloading", "no exception", e.what());
        return false;
    }

    TestOutput::PrintTestPass("model watching and reloading");
    return true;
}

bool TestModelDevelopmentToolsIntegration() {
    TestOutput::PrintTestStart("ModelDevelopmentTools integration");

    try {
        auto modelLoader = std::make_shared<ModelLoader>();
        if (!modelLoader->Initialize()) {
            TestOutput::PrintTestFail("ModelDevelopmentTools integration - ModelLoader initialization failed");
            return false;
        }

        ModelDevelopmentTools devTools;
        if (!devTools.Initialize(modelLoader)) {
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("ModelDevelopmentTools integration", "true", "false");
            return false;
        }

        if (!devTools.IsInitialized()) {
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("ModelDevelopmentTools integration", "initialized", "not initialized");
            return false;
        }

        // Test configuration
        ModelDevelopmentTools::DevelopmentConfig config;
        config.enableHotReloading = true;
        config.enableValidation = true;
        config.hotReloadInterval = std::chrono::milliseconds(200);
        devTools.SetConfig(config);

        auto retrievedConfig = devTools.GetConfig();
        if (!retrievedConfig.enableHotReloading) {
            devTools.Shutdown();
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("ModelDevelopmentTools integration", "hot-reloading enabled", "disabled");
            return false;
        }

        // Test performance metrics
        auto metrics = devTools.GetPerformanceMetrics();
        if (metrics.totalModelsLoaded != 0) {
            devTools.Shutdown();
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("ModelDevelopmentTools integration", "0 models loaded initially", 
                                    std::to_string(metrics.totalModelsLoaded));
            return false;
        }

        devTools.Shutdown();
        modelLoader->Shutdown();

    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("ModelDevelopmentTools integration", "no exception", e.what());
        return false;
    }

    TestOutput::PrintTestPass("ModelDevelopmentTools integration");
    return true;
}

bool TestModelValidation() {
    TestOutput::PrintTestStart("model validation");

    try {
        // Create a test model file
        std::string testFilePath = "test_validation_model.obj";
        if (!CreateTestModelFile(testFilePath, TEST_OBJ_CONTENT_V1)) {
            TestOutput::PrintTestFail("model validation", "test file created", "failed to create test file");
            return false;
        }

        auto modelLoader = std::make_shared<ModelLoader>();
        if (!modelLoader->Initialize()) {
            std::filesystem::remove(testFilePath);
            TestOutput::PrintTestFail("model validation - ModelLoader initialization failed");
            return false;
        }

        ModelDevelopmentTools devTools;
        if (!devTools.Initialize(modelLoader)) {
            std::filesystem::remove(testFilePath);
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("model validation - DevTools initialization failed");
            return false;
        }

        // Test file validation
        auto fileValidation = devTools.ValidateModelFile(testFilePath);
        if (!fileValidation.isValid) {
            std::filesystem::remove(testFilePath);
            devTools.Shutdown();
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("model validation", "valid model", "validation failed");
            return false;
        }

        if (fileValidation.vertexCount == 0) {
            std::filesystem::remove(testFilePath);
            devTools.Shutdown();
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("model validation", "vertices > 0", "0 vertices");
            return false;
        }

        // Test model validation
        auto model = modelLoader->LoadModelAsResource(testFilePath);
        if (model) {
            auto modelValidation = devTools.ValidateModel(model);
            if (!modelValidation.isValid) {
                std::filesystem::remove(testFilePath);
                devTools.Shutdown();
                modelLoader->Shutdown();
                TestOutput::PrintTestFail("model validation", "valid loaded model", "validation failed");
                return false;
            }
        }

        // Cleanup
        std::filesystem::remove(testFilePath);
        devTools.Shutdown();
        modelLoader->Shutdown();

    } catch (const std::exception& e) {
        std::filesystem::remove("test_validation_model.obj");
        TestOutput::PrintTestFail("model validation", "no exception", e.what());
        return false;
    }

    TestOutput::PrintTestPass("model validation");
    return true;
}

bool TestHotReloadPerformance() {
    TestOutput::PrintTestStart("hot-reload performance");

    try {
        // Create test file
        std::string testFilePath = "test_performance_model.obj";
        if (!CreateTestModelFile(testFilePath, TEST_OBJ_CONTENT_V1)) {
            TestOutput::PrintTestFail("hot-reload performance", "test file created", "failed to create test file");
            return false;
        }

        auto modelLoader = std::make_shared<ModelLoader>();
        if (!modelLoader->Initialize()) {
            std::filesystem::remove(testFilePath);
            TestOutput::PrintTestFail("hot-reload performance - ModelLoader initialization failed");
            return false;
        }

        ModelDevelopmentTools devTools;
        if (!devTools.Initialize(modelLoader)) {
            std::filesystem::remove(testFilePath);
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("hot-reload performance - DevTools initialization failed");
            return false;
        }

        // Load model and start watching
        auto model = modelLoader->LoadModelAsResource(testFilePath);
        if (!model) {
            std::filesystem::remove(testFilePath);
            devTools.Shutdown();
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("hot-reload performance", "model loaded", "model is null");
            return false;
        }

        devTools.WatchModel(testFilePath, model);
        devTools.EnableHotReloading();

        // Measure reload time
        auto startTime = std::chrono::high_resolution_clock::now();
        
        // Trigger manual reload
        devTools.ReloadAllWatchedModels();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        float reloadTime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

        // Check that reload completed in reasonable time (< 1 second)
        if (reloadTime > 1000.0f) {
            std::filesystem::remove(testFilePath);
            devTools.Shutdown();
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("hot-reload performance", "reload time < 1000ms", 
                                    std::to_string(reloadTime) + "ms");
            return false;
        }

        // Check performance metrics
        auto metrics = devTools.GetPerformanceMetrics();
        if (metrics.totalReloads == 0) {
            std::filesystem::remove(testFilePath);
            devTools.Shutdown();
            modelLoader->Shutdown();
            TestOutput::PrintTestFail("hot-reload performance", "reloads > 0", "0 reloads");
            return false;
        }

        TestOutput::PrintInfo("Reload completed in " + std::to_string(static_cast<int>(reloadTime)) + "ms");

        // Cleanup
        std::filesystem::remove(testFilePath);
        devTools.Shutdown();
        modelLoader->Shutdown();

    } catch (const std::exception& e) {
        std::filesystem::remove("test_performance_model.obj");
        TestOutput::PrintTestFail("hot-reload performance", "no exception", e.what());
        return false;
    }

    TestOutput::PrintTestPass("hot-reload performance");
    return true;
}

int main() {
    TestOutput::PrintHeader("Model Hot-Reload Integration Tests");

    TestSuite suite("Model Hot-Reload Integration Tests");

    try {
        suite.RunTest("ModelHotReloader Initialization", TestModelHotReloaderInitialization);
        suite.RunTest("Model Watching and Reloading", TestModelWatchingAndReloading);
        suite.RunTest("ModelDevelopmentTools Integration", TestModelDevelopmentToolsIntegration);
        suite.RunTest("Model Validation", TestModelValidation);
        suite.RunTest("Hot-Reload Performance", TestHotReloadPerformance);

        suite.PrintSummary();
        return suite.AllTestsPassed() ? 0 : 1;

    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception: " + std::string(e.what()));
        return 1;
    }
}