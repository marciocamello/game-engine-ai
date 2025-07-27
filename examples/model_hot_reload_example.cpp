/**
 * @file model_hot_reload_example.cpp
 * @brief Example demonstrating model hot-reloading during development
 * 
 * This example shows how to set up and use the model hot-reloading system
 * for seamless development workflow with real-time model updates.
 */

#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/Model.h"
#include "Resource/ModelLoader.h"
#include "Resource/ModelDevelopmentTools.h"
#include "Resource/ResourceManager.h"

#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace GameEngine;

class ModelHotReloadDemo {
public:
    bool Initialize() {
        // Initialize engine
        m_engine = std::make_unique<Engine>();
        if (!m_engine->Initialize()) {
            Logger::GetInstance().Error("Failed to initialize engine");
            return false;
        }

        // Get core systems - using raw pointers as returned by engine
        auto renderer = m_engine->GetRenderer();
        auto resourceManager = m_engine->GetResourceManager();
        
        // Convert to shared_ptr for our use
        m_renderer = std::shared_ptr<GraphicsRenderer>(renderer, [](GraphicsRenderer*){});
        m_resourceManager = std::shared_ptr<ResourceManager>(resourceManager, [](ResourceManager*){});


        if (!m_renderer || !m_resourceManager) {
            Logger::GetInstance().Error("Failed to get engine systems");
            return false;
        }

        // Initialize model loader
        m_modelLoader = std::make_shared<ModelLoader>();
        if (!m_modelLoader->Initialize()) {
            Logger::GetInstance().Error("Failed to initialize ModelLoader");
            return false;
        }

        // Initialize development tools
        m_devTools = std::make_shared<ModelDevelopmentTools>();
        if (!m_devTools->Initialize(m_modelLoader, m_resourceManager)) {
            Logger::GetInstance().Error("Failed to initialize ModelDevelopmentTools");
            return false;
        }

        // Configure development tools for hot-reloading
        ModelDevelopmentTools::DevelopmentConfig config;
        config.enableHotReloading = true;
        config.enableValidation = true;
        config.enableOptimization = true;
        config.hotReloadInterval = std::chrono::milliseconds(250); // Fast polling for demo
        config.assetDirectories = {"assets/meshes", "assets/GLTF", "assets/models"};
        m_devTools->SetConfig(config);

        // Set up reload callback
        m_devTools->SetReloadCallback([this](const std::string& path, std::shared_ptr<Model> newModel, bool success) {
            OnModelReloaded(path, newModel, success);
        });

        // Load initial models
        LoadDemoModels();

        // Enable hot-reloading
        m_devTools->EnableHotReloading();

        Logger::GetInstance().Info("Model Hot-Reload Demo initialized successfully");
        Logger::GetInstance().Info("Demo will run automatically and show hot-reloading functionality");
        
        return true;
    }

    void Run() {
        Logger::GetInstance().Info("Starting Model Hot-Reload Demo");
        Logger::GetInstance().Info("Controls:");
        Logger::GetInstance().Info("  ESC - Exit");
        Logger::GetInstance().Info("  R - Reload all models manually");
        Logger::GetInstance().Info("  V - Validate all models");
        Logger::GetInstance().Info("  O - Optimize all models");
        Logger::GetInstance().Info("  S - Show status");
        Logger::GetInstance().Info("  P - Performance report");

        while (m_running) {
            ProcessInput();
            Update();
            Render();
            
            // Small delay to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
        }
    }

    void Shutdown() {
        if (m_devTools) {
            m_devTools->DisableHotReloading();
            m_devTools->Shutdown();
        }

        if (m_modelLoader) {
            m_modelLoader->Shutdown();
        }

        if (m_engine) {
            m_engine->Shutdown();
        }

        Logger::GetInstance().Info("Model Hot-Reload Demo shutdown complete");
    }

private:
    void LoadDemoModels() {
        // List of demo models to load and watch
        std::vector<std::string> demoModels = {
            "assets/meshes/cube.obj",
            "assets/meshes/sphere.obj",
            "assets/meshes/teapot.obj",
            "assets/GLTF/simple_scene.gltf",
            "assets/meshes/XBot.fbx"
        };

        Logger::GetInstance().Info("Loading demo models...");

        for (const auto& modelPath : demoModels) {
            try {
                if (std::filesystem::exists(modelPath)) {
                    auto model = m_modelLoader->LoadModelAsResource(modelPath);
                    if (model) {
                        m_loadedModels[modelPath] = model;
                        
                        // Start watching this model for changes
                        m_devTools->WatchModel(modelPath, model);
                        
                        // Print model info
                        m_devTools->PrintModelInfo(model);
                        
                        Logger::GetInstance().Info("Loaded and watching: " + modelPath);
                    } else {
                        Logger::GetInstance().Warning("Failed to load model: " + modelPath);
                    }
                } else {
                    Logger::GetInstance().Info("Demo model not found (skipping): " + modelPath);
                }
            } catch (const std::exception& e) {
                Logger::GetInstance().Error("Exception loading " + modelPath + ": " + e.what());
            }
        }

        Logger::GetInstance().Info("Loaded " + std::to_string(m_loadedModels.size()) + " demo models");
    }

    void ProcessInput() {
        // Input processing simplified for demo - in real app would use proper input system
        // For now, just run for a limited time then exit
        static int frameCount = 0;
        frameCount++;
        
        // Run for 100 frames then exit
        if (frameCount > 100) {
            m_running = false;
            Logger::GetInstance().Info("Demo completed after 100 frames");
        }
        
        // Trigger some operations periodically for demo
        if (frameCount == 20) {
            Logger::GetInstance().Info("Manual reload requested");
            m_devTools->ReloadAllWatchedModels();
        }
        
        if (frameCount == 40) {
            Logger::GetInstance().Info("Validation requested");
            m_devTools->ValidateAllWatchedModels();
        }
        
        if (frameCount == 60) {
            Logger::GetInstance().Info("Optimization requested");
            m_devTools->OptimizeAllWatchedModels();
        }
        
        if (frameCount == 80) {
            ShowStatus();
        }
    }

    void Update() {
        // Engine update is handled internally
        // Just handle our demo logic here
        
        // Periodically show status (every 10 seconds)
        static auto lastStatusTime = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (now - lastStatusTime > std::chrono::seconds(10)) {
            ShowQuickStatus();
            lastStatusTime = now;
        }
    }

    void Render() {
        if (!m_renderer) return;

        m_renderer->BeginFrame();
        
        // Render loaded models (simplified rendering)
        for (const auto& pair : m_loadedModels) {
            const auto& model = pair.second;
            if (model) {
                // Simple rendering - in a real application you'd have proper scene management
                Math::Mat4 transform = Math::Mat4(1.0f); // Identity matrix
                // model->Render(transform, shader); // Would need a shader
            }
        }
        
        m_renderer->EndFrame();
    }

    void OnModelReloaded(const std::string& modelPath, std::shared_ptr<Model> newModel, bool success) {
        if (success && newModel) {
            // Update our model reference
            m_loadedModels[modelPath] = newModel;
            
            Logger::GetInstance().Info("=== Model Reloaded Successfully ===");
            Logger::GetInstance().Info("Path: " + modelPath);
            
            // Print updated model info
            m_devTools->PrintModelInfo(newModel);
            
            // Validate the new model
            auto validation = m_devTools->ValidateModel(newModel);
            if (validation.isValid) {
                Logger::GetInstance().Info("Reloaded model passed validation");
            } else {
                Logger::GetInstance().Warning("Reloaded model has validation issues:");
                for (const auto& error : validation.errors) {
                    Logger::GetInstance().Warning("  Error: " + error);
                }
                for (const auto& warning : validation.warnings) {
                    Logger::GetInstance().Warning("  Warning: " + warning);
                }
            }
            
        } else {
            Logger::GetInstance().Error("=== Model Reload Failed ===");
            Logger::GetInstance().Error("Path: " + modelPath);
        }
    }

    void ShowStatus() {
        Logger::GetInstance().Info("=== Model Hot-Reload Demo Status ===");
        Logger::GetInstance().Info("Running: " + std::string(m_running ? "Yes" : "No"));
        Logger::GetInstance().Info("Hot-reloading enabled: " + std::string(m_devTools->IsHotReloadingEnabled() ? "Yes" : "No"));
        Logger::GetInstance().Info("Loaded models: " + std::to_string(m_loadedModels.size()));
        
        // Show watched models
        m_devTools->PrintWatchedModelsStatus();
        
        // Show asset directory status
        m_devTools->PrintAssetDirectoryStatus();
        
        // Show performance metrics
        m_devTools->LogPerformanceReport();
    }

    void ShowQuickStatus() {
        auto metrics = m_devTools->GetPerformanceMetrics();
        Logger::GetInstance().Info("Quick Status - Models: " + std::to_string(m_loadedModels.size()) + 
                                  ", Reloads: " + std::to_string(metrics.totalReloads) + 
                                  ", Hot-reload: " + (m_devTools->IsHotReloadingEnabled() ? "ON" : "OFF"));
    }

private:
    // Core engine systems
    std::unique_ptr<Engine> m_engine;
    std::shared_ptr<GraphicsRenderer> m_renderer;
    std::shared_ptr<ResourceManager> m_resourceManager;

    // Model systems
    std::shared_ptr<ModelLoader> m_modelLoader;
    std::shared_ptr<ModelDevelopmentTools> m_devTools;

    // Demo state
    std::unordered_map<std::string, std::shared_ptr<Model>> m_loadedModels;
    bool m_running = true;
};

int main() {
    try {
        ModelHotReloadDemo demo;
        
        if (!demo.Initialize()) {
            std::cerr << "Failed to initialize Model Hot-Reload Demo" << std::endl;
            return 1;
        }
        
        demo.Run();
        demo.Shutdown();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in Model Hot-Reload Demo: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception in Model Hot-Reload Demo" << std::endl;
        return 1;
    }
}