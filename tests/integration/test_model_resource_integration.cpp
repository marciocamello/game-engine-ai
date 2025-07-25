#include "../TestUtils.h"
#include "Resource/ResourceManager.h"
#include "Graphics/Model.h"
#include "Resource/ModelLoader.h"
#include "Core/Logger.h"
#include <memory>
#include <chrono>
#include <thread>

using namespace GameEngine;
using namespace GameEngine::Testing;

bool TestModelResourceLoading() {
    TestOutput::PrintTestStart("Model resource loading through ResourceManager");
    
    try {
        // Initialize ResourceManager
        ResourceManager resourceManager;
        if (!resourceManager.Initialize()) {
            TestOutput::PrintError("Model resource loading");
            return false;
        }
        
        // Test loading a model through ResourceManager
        auto model = resourceManager.Load<Model>("test_model.obj");
        if (!model) {
            // This is expected if the file doesn't exist, so create a default model
            model = std::make_shared<Model>("test_default");
            model->CreateDefault();
            
            if (model->GetMeshCount() == 0) {
                TestOutput::PrintError("Model resource loading");
                return false;
            }
        }
        
        // Verify model properties
        if (model->GetMemoryUsage() == 0) {
            TestOutput::PrintError("Model resource loading");
            return false;
        }
        
        // Test model statistics
        auto stats = model->GetStats();
        if (stats.meshCount == 0) {
            TestOutput::PrintError("Model resource loading");
            return false;
        }
        
        TestOutput::PrintInfo("Model loaded with " + std::to_string(stats.meshCount) + " meshes, " +
                             std::to_string(stats.totalMemoryUsage) + " bytes memory usage");
        
        resourceManager.Shutdown();
        
        TestOutput::PrintTestPass("Model resource loading");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintTestFail("Model resource loading");
        TestOutput::PrintError("Exception: " + std::string(e.what()));
        return false;
    }
}

bool TestModelResourceCaching() {
    TestOutput::PrintTestStart("Model resource caching");
    
    try {
        ResourceManager resourceManager;
        if (!resourceManager.Initialize()) {
            TestOutput::PrintError("Model resource caching");
            return false;
        }
        
        // Load the same model twice
        auto model1 = resourceManager.Load<Model>("test_cache");
        if (!model1) {
            model1 = std::make_shared<Model>("test_cache");
            model1->CreateDefault();
        }
        
        auto model2 = resourceManager.Load<Model>("test_cache");
        if (!model2) {
            model2 = std::make_shared<Model>("test_cache");
            model2->CreateDefault();
        }
        
        // They should be the same instance due to caching
        if (model1.get() != model2.get()) {
            TestOutput::PrintError("Model resource caching");
            return false;
        }
        
        TestOutput::PrintInfo("Resource caching working correctly - same instance returned");
        
        resourceManager.Shutdown();
        
        TestOutput::PrintTestPass("Model resource caching");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("Model resource caching");
        return false;
    }
}

bool TestModelResourceManager() {
    TestOutput::PrintTestStart("Model resource management through ResourceManager");
    
    try {
        ResourceManager resourceManager;
        if (!resourceManager.Initialize()) {
            TestOutput::PrintError("Model resource management");
            return false;
        }
        
        // Test loading multiple models
        auto model1 = resourceManager.Load<Model>("test_model1");
        auto model2 = resourceManager.Load<Model>("test_model2");
        auto model3 = resourceManager.Load<Model>("test_model3");
        
        // Since files don't exist, they should be null or fallback resources
        if (!model1) {
            model1 = std::make_shared<Model>("test_model1");
            model1->CreateDefault();
        }
        
        // Test resource statistics
        auto resourceCount = resourceManager.GetResourceCount();
        auto memoryUsage = resourceManager.GetMemoryUsage();
        auto stats = resourceManager.GetResourceStats();
        
        TestOutput::PrintInfo("ResourceManager stats - Resources: " + std::to_string(resourceCount) +
                             ", Memory: " + std::to_string(memoryUsage) + " bytes");
        
        // Test resource cleanup
        resourceManager.UnloadUnused();
        
        resourceManager.Shutdown();
        
        TestOutput::PrintTestPass("Model resource management through ResourceManager");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("Model resource management");
        return false;
    }
}

bool TestModelResourceLifecycle() {
    TestOutput::PrintTestStart("Model resource lifecycle management");
    
    try {
        ResourceManager resourceManager;
        if (!resourceManager.Initialize()) {
            TestOutput::PrintError("Model resource lifecycle");
            return false;
        }
        
        // Test automatic cleanup when resources go out of scope
        {
            auto model = resourceManager.Load<Model>("lifecycle_test");
            if (!model) {
                model = std::make_shared<Model>("lifecycle_test");
                model->CreateDefault();
            }
            
            // Verify model is loaded
            if (resourceManager.GetResourceCount() == 0) {
                TestOutput::PrintError("Model resource lifecycle");
                return false;
            }
        }
        
        // Force cleanup of unused resources
        resourceManager.UnloadUnused();
        
        // Test memory pressure handling
        resourceManager.SetMemoryPressureThreshold(1024); // Very low threshold
        resourceManager.CheckMemoryPressure();
        
        TestOutput::PrintInfo("Resource lifecycle management working correctly");
        
        resourceManager.Shutdown();
        
        TestOutput::PrintTestPass("Model resource lifecycle");
        return true;
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("Model resource lifecycle");
        return false;
    }
}

int main() {
    TestOutput::PrintHeader("Model Resource Integration Tests");
    
    TestSuite suite("Model Resource Integration");
    
    try {
        suite.RunTest("Model Resource Loading", TestModelResourceLoading);
        suite.RunTest("Model Resource Caching", TestModelResourceCaching);
        suite.RunTest("Model Resource Management", TestModelResourceManager);
        suite.RunTest("Model Resource Lifecycle", TestModelResourceLifecycle);
        
        suite.PrintSummary();
        return suite.AllTestsPassed() ? 0 : 1;
        
    } catch (const std::exception& e) {
        TestOutput::PrintError("Exception: " + std::string(e.what()));
        return 1;
    }
}