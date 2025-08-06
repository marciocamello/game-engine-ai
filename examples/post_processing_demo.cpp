// Post-Processing Effects Demonstration
// This example showcases the post-processing pipeline capabilities including:
// - Tone mapping effects (Reinhard, ACES, Filmic)
// - Anti-aliasing (FXAA) with quality settings
// - Bloom effects with configurable parameters
// - Interactive effect controls and real-time adjustments

#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Graphics/Camera.h"
#include "Graphics/GraphicsRenderer.h"
#include "Graphics/OpenGLRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Graphics/GridRenderer.h"
#include "Graphics/Shader.h"
#include "Graphics/Material.h"
#include "Graphics/Texture.h"
#include "Input/InputManager.h"
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>

using namespace GameEngine;

/**
 * @brief Camera for post-processing demonstration
 */
class PostProcessDemoCamera : public Camera {
public:
    PostProcessDemoCamera() {
        SetPosition(Math::Vec3(0.0f, 8.0f, 20.0f));
        m_yaw = -90.0f;
        m_pitch = -15.0f;
        m_moveSpeed = 10.0f;
        m_mouseSensitivity = 0.1f;
        UpdateCameraVectors();
    }

    void Update(float deltaTime, InputManager* input) {
        // Movement with WASD + E/Q
        Math::Vec3 velocity(0.0f);
        
        if (input->IsActionDown("move_forward")) {
            velocity += m_front;
        }
        if (input->IsActionDown("move_backward")) {
            velocity -= m_front;
        }
        if (input->IsActionDown("move_left")) {
            velocity -= m_right;
        }
        if (input->IsActionDown("move_right")) {
            velocity += m_right;
        }
        if (input->IsActionDown("move_up")) {
            velocity += m_worldUp;
        }
        if (input->IsActionDown("move_down")) {
            velocity -= m_worldUp;
        }

        // Apply movement
        if (velocity.x != 0.0f || velocity.y != 0.0f || velocity.z != 0.0f) {
            Math::Vec3 normalizedVelocity = glm::normalize(velocity);
            Math::Vec3 currentPos = GetPosition();
            SetPosition(currentPos + normalizedVelocity * m_moveSpeed * deltaTime);
        }

        // Mouse look
        auto mouseDelta = input->GetMouseDelta();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
            m_yaw += mouseDelta.x * m_mouseSensitivity;
            m_pitch -= mouseDelta.y * m_mouseSensitivity;

            // Constrain pitch
            if (m_pitch > 89.0f) m_pitch = 89.0f;
            if (m_pitch < -89.0f) m_pitch = -89.0f;

            UpdateCameraVectors();
        }
    }

private:
    void UpdateCameraVectors() {
        Math::Vec3 front;
        front.x = cosf(Math::ToRadians(m_yaw)) * cosf(Math::ToRadians(m_pitch));
        front.y = sinf(Math::ToRadians(m_pitch));
        front.z = sinf(Math::ToRadians(m_yaw)) * cosf(Math::ToRadians(m_pitch));
        
        m_front = glm::normalize(front);
        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
        
        // Update camera rotation using GLM lookAt
        Math::Vec3 currentPos = GetPosition();
        LookAt(currentPos + m_front, m_up);
    }

    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_moveSpeed = 10.0f;
    float m_mouseSensitivity = 0.1f;
    
    Math::Vec3 m_front{0.0f, 0.0f, -1.0f};
    Math::Vec3 m_up{0.0f, 1.0f, 0.0f};
    Math::Vec3 m_right{1.0f, 0.0f, 0.0f};
    Math::Vec3 m_worldUp{0.0f, 1.0f, 0.0f};
};

/**
 * @brief HDR scene object for demonstrating post-processing effects
 */
struct HDRSceneObject {
    Math::Vec3 position;
    Math::Vec3 scale;
    Math::Vec4 color;
    float emissionStrength;
    bool isEmissive;
};

class PostProcessingDemoApplication {
public:
    enum class ToneMappingType {
        None,
        Reinhard,
        ACES,
        Filmic
    };

    PostProcessingDemoApplication() = default;
    ~PostProcessingDemoApplication() {
        LOG_INFO("PostProcessingDemoApplication cleaned up successfully");
    }

    bool Initialize() {
        if (!m_engine.Initialize()) {
            LOG_ERROR("Failed to initialize game engine");
            return false;
        }

        m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
        if (!m_primitiveRenderer->Initialize()) {
            LOG_ERROR("Failed to initialize primitive renderer");
            return false;
        }

        // Initialize camera for post-processing demonstration
        m_camera = std::make_unique<PostProcessDemoCamera>();
        m_engine.GetRenderer()->SetCamera(m_camera.get());
        m_engine.SetMainCamera(m_camera.get());
        
        LOG_INFO("Post-processing demo camera initialized");

        // Initialize professional grid renderer
        m_gridRenderer = std::make_unique<GridRenderer>();
        if (!m_gridRenderer->Initialize(m_primitiveRenderer.get())) {
            LOG_ERROR("Failed to initialize grid renderer");
            return false;
        }

        // Create HDR scene for post-processing demonstration
        CreateHDRScene();

        // Setup high-intensity lighting for HDR demonstration
        SetupHDRLighting();

        // Bind controls
        BindControls();

        m_engine.SetUpdateCallback([this](float deltaTime) { this->Update(deltaTime); });
        m_engine.SetRenderCallback([this]() { this->Render(); });

        PrintWelcomeMessage();
        return true;
    }

    void Run() {
        LOG_INFO("Starting post-processing effects demonstration...");
        m_engine.Run();
    }

private:
    void CreateHDRScene() {
        m_sceneObjects.clear();

        // Create bright emissive objects to demonstrate HDR and bloom
        
        // Bright red emissive cube
        HDRSceneObject redEmissive;
        redEmissive.position = Math::Vec3(-8.0f, 3.0f, 0.0f);
        redEmissive.scale = Math::Vec3(2.0f);
        redEmissive.color = Math::Vec4(5.0f, 0.5f, 0.5f, 1.0f); // HDR red
        redEmissive.emissionStrength = 3.0f;
        redEmissive.isEmissive = true;
        m_sceneObjects.push_back(redEmissive);

        // Bright green emissive cube
        HDRSceneObject greenEmissive;
        greenEmissive.position = Math::Vec3(-4.0f, 3.0f, 0.0f);
        greenEmissive.scale = Math::Vec3(2.0f);
        greenEmissive.color = Math::Vec4(0.5f, 5.0f, 0.5f, 1.0f); // HDR green
        greenEmissive.emissionStrength = 3.0f;
        greenEmissive.isEmissive = true;
        m_sceneObjects.push_back(greenEmissive);

        // Bright blue emissive cube
        HDRSceneObject blueEmissive;
        blueEmissive.position = Math::Vec3(0.0f, 3.0f, 0.0f);
        blueEmissive.scale = Math::Vec3(2.0f);
        blueEmissive.color = Math::Vec4(0.5f, 0.5f, 5.0f, 1.0f); // HDR blue
        blueEmissive.emissionStrength = 3.0f;
        blueEmissive.isEmissive = true;
        m_sceneObjects.push_back(blueEmissive);

        // Very bright white emissive cube (for bloom demonstration)
        HDRSceneObject whiteEmissive;
        whiteEmissive.position = Math::Vec3(4.0f, 3.0f, 0.0f);
        whiteEmissive.scale = Math::Vec3(2.0f);
        whiteEmissive.color = Math::Vec4(8.0f, 8.0f, 8.0f, 1.0f); // Very bright white
        whiteEmissive.emissionStrength = 5.0f;
        whiteEmissive.isEmissive = true;
        m_sceneObjects.push_back(whiteEmissive);

        // Normal diffuse objects for comparison
        HDRSceneObject normalCube1;
        normalCube1.position = Math::Vec3(-6.0f, 1.0f, -4.0f);
        normalCube1.scale = Math::Vec3(1.5f);
        normalCube1.color = Math::Vec4(0.8f, 0.2f, 0.2f, 1.0f); // Normal red
        normalCube1.emissionStrength = 0.0f;
        normalCube1.isEmissive = false;
        m_sceneObjects.push_back(normalCube1);

        HDRSceneObject normalCube2;
        normalCube2.position = Math::Vec3(-2.0f, 1.0f, -4.0f);
        normalCube2.scale = Math::Vec3(1.5f);
        normalCube2.color = Math::Vec4(0.2f, 0.8f, 0.2f, 1.0f); // Normal green
        normalCube2.emissionStrength = 0.0f;
        normalCube2.isEmissive = false;
        m_sceneObjects.push_back(normalCube2);

        HDRSceneObject normalCube3;
        normalCube3.position = Math::Vec3(2.0f, 1.0f, -4.0f);
        normalCube3.scale = Math::Vec3(1.5f);
        normalCube3.color = Math::Vec4(0.2f, 0.2f, 0.8f, 1.0f); // Normal blue
        normalCube3.emissionStrength = 0.0f;
        normalCube3.isEmissive = false;
        m_sceneObjects.push_back(normalCube3);

        LOG_INFO("POST-PROCESSING DEMO: Created HDR scene with " + std::to_string(m_sceneObjects.size()) + " objects");
        LOG_INFO("  - 4 bright emissive objects for HDR/bloom demonstration");
        LOG_INFO("  - 3 normal diffuse objects for comparison");
    }

    void SetupHDRLighting() {
        auto* openglRenderer = static_cast<OpenGLRenderer*>(m_engine.GetRenderer());
        if (!openglRenderer) return;

        // Setup bright directional light for HDR
        m_lightDirection = Math::Vec3(-0.3f, -1.0f, -0.2f);
        m_lightDirection = glm::normalize(m_lightDirection);
        m_lightColor = Math::Vec3(2.0f, 1.8f, 1.5f); // Bright warm light
        m_lightIntensity = 4.0f; // High intensity for HDR

        openglRenderer->SetDirectionalLight(m_lightDirection, m_lightColor, m_lightIntensity);

        // Add additional point lights for complex lighting
        openglRenderer->AddPointLight(Math::Vec3(-10.0f, 8.0f, 5.0f), Math::Vec3(3.0f, 2.0f, 1.0f), 8.0f, 20.0f);
        openglRenderer->AddPointLight(Math::Vec3(10.0f, 8.0f, 5.0f), Math::Vec3(1.0f, 2.0f, 3.0f), 8.0f, 20.0f);

        LOG_INFO("POST-PROCESSING DEMO: HDR lighting setup complete");
        LOG_INFO("  - High-intensity directional light");
        LOG_INFO("  - Multiple bright point lights");
        LOG_INFO("  - Scene designed for HDR and bloom effects");
    }

    void BindControls() {
        auto *input = m_engine.GetInput();
        
        // Navigation controls
        input->BindAction("move_forward", KeyCode::W);
        input->BindAction("move_backward", KeyCode::S);
        input->BindAction("move_left", KeyCode::A);
        input->BindAction("move_right", KeyCode::D);
        input->BindAction("move_up", KeyCode::E);
        input->BindAction("move_down", KeyCode::Q);
        input->BindAction("quit", KeyCode::Escape);
        
        LOG_INFO("POST-PROCESSING DEMO: Controls bound successfully");
    }

    void Update(float deltaTime) {
        auto *input = m_engine.GetInput();
        auto *window = m_engine.GetRenderer()->GetWindow();

        // Mouse capture toggle
        if (input->IsKeyPressed(KeyCode::Escape)) {
            static bool mouseCaptured = true;
            mouseCaptured = !mouseCaptured;
            if (mouseCaptured) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                LOG_INFO("Mouse captured for navigation");
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                LOG_INFO("Mouse released");
            }
        }

        // Exit application
        if (input->IsKeyPressed(KeyCode::F1)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            LOG_INFO("Exiting post-processing demonstration");
            return;
        }

        // Post-processing effect controls
        HandlePostProcessingControls(input);

        // HDR and exposure controls
        HandleHDRControls(input);

        // Update camera
        m_camera->Update(deltaTime, m_engine.GetInput());
        
        // Sync renderer with primitive renderer
        auto* openglRenderer = static_cast<OpenGLRenderer*>(m_engine.GetRenderer());
        if (openglRenderer) {
            openglRenderer->SyncWithPrimitiveRenderer(m_primitiveRenderer.get());
        }
    }

    void HandlePostProcessingControls(InputManager* input) {
        // F2 - Cycle tone mapping operators
        if (input->IsKeyPressed(KeyCode::F2)) {
            CycleToneMappingOperator();
        }

        // F3 - Toggle FXAA anti-aliasing
        if (input->IsKeyPressed(KeyCode::F3)) {
            m_fxaaEnabled = !m_fxaaEnabled;
            LOG_INFO("POST-PROCESSING DEMO: FXAA " + std::string(m_fxaaEnabled ? "ENABLED" : "DISABLED"));
            if (m_fxaaEnabled) {
                LOG_INFO("  - Anti-aliasing quality: " + std::to_string(m_fxaaQuality));
            }
        }

        // F4 - Adjust FXAA quality
        if (input->IsKeyPressed(KeyCode::F4) && m_fxaaEnabled) {
            m_fxaaQuality += 0.25f;
            if (m_fxaaQuality > 1.0f) m_fxaaQuality = 0.25f;
            LOG_INFO("POST-PROCESSING DEMO: FXAA quality set to " + std::to_string(m_fxaaQuality));
        }

        // F5 - Toggle bloom effect
        if (input->IsKeyPressed(KeyCode::F5)) {
            m_bloomEnabled = !m_bloomEnabled;
            LOG_INFO("POST-PROCESSING DEMO: Bloom " + std::string(m_bloomEnabled ? "ENABLED" : "DISABLED"));
            if (m_bloomEnabled) {
                LOG_INFO("  - Bloom threshold: " + std::to_string(m_bloomThreshold));
                LOG_INFO("  - Bloom intensity: " + std::to_string(m_bloomIntensity));
                LOG_INFO("  - Bloom radius: " + std::to_string(m_bloomRadius));
            }
        }

        // F6/F7 - Adjust bloom threshold
        if (input->IsKeyPressed(KeyCode::F6) && m_bloomEnabled) {
            m_bloomThreshold -= 0.1f;
            if (m_bloomThreshold < 0.1f) m_bloomThreshold = 0.1f;
            LOG_INFO("POST-PROCESSING DEMO: Bloom threshold decreased to " + std::to_string(m_bloomThreshold));
        }
        if (input->IsKeyPressed(KeyCode::F7) && m_bloomEnabled) {
            m_bloomThreshold += 0.1f;
            if (m_bloomThreshold > 3.0f) m_bloomThreshold = 3.0f;
            LOG_INFO("POST-PROCESSING DEMO: Bloom threshold increased to " + std::to_string(m_bloomThreshold));
        }

        // F8/F9 - Adjust bloom intensity
        if (input->IsKeyPressed(KeyCode::F8) && m_bloomEnabled) {
            m_bloomIntensity -= 0.1f;
            if (m_bloomIntensity < 0.1f) m_bloomIntensity = 0.1f;
            LOG_INFO("POST-PROCESSING DEMO: Bloom intensity decreased to " + std::to_string(m_bloomIntensity));
        }
        if (input->IsKeyPressed(KeyCode::F9) && m_bloomEnabled) {
            m_bloomIntensity += 0.1f;
            if (m_bloomIntensity > 2.0f) m_bloomIntensity = 2.0f;
            LOG_INFO("POST-PROCESSING DEMO: Bloom intensity increased to " + std::to_string(m_bloomIntensity));
        }

        // F10 - Show current post-processing settings
        if (input->IsKeyPressed(KeyCode::F10)) {
            ShowPostProcessingSettings();
        }
    }

    void HandleHDRControls(InputManager* input) {
        // F11/F12 - Adjust exposure
        if (input->IsKeyPressed(KeyCode::F11)) {
            m_exposure -= 0.2f;
            if (m_exposure < 0.1f) m_exposure = 0.1f;
            LOG_INFO("POST-PROCESSING DEMO: Exposure decreased to " + std::to_string(m_exposure));
        }
        if (input->IsKeyPressed(KeyCode::F12)) {
            m_exposure += 0.2f;
            if (m_exposure > 5.0f) m_exposure = 5.0f;
            LOG_INFO("POST-PROCESSING DEMO: Exposure increased to " + std::to_string(m_exposure));
        }

        // G - Adjust gamma
        if (input->IsKeyPressed(KeyCode::G)) {
            m_gamma += 0.1f;
            if (m_gamma > 3.0f) m_gamma = 1.8f; // Cycle back
            LOG_INFO("POST-PROCESSING DEMO: Gamma set to " + std::to_string(m_gamma));
        }
    }

    void CycleToneMappingOperator() {
        int currentType = static_cast<int>(m_toneMappingType);
        currentType = (currentType + 1) % 4;
        m_toneMappingType = static_cast<ToneMappingType>(currentType);

        std::string typeName;
        switch (m_toneMappingType) {
            case ToneMappingType::None:
                typeName = "None (Linear)";
                break;
            case ToneMappingType::Reinhard:
                typeName = "Reinhard";
                break;
            case ToneMappingType::ACES:
                typeName = "ACES Filmic";
                break;
            case ToneMappingType::Filmic:
                typeName = "Uncharted 2 Filmic";
                break;
        }

        LOG_INFO("POST-PROCESSING DEMO: Tone mapping operator changed to " + typeName);
    }

    void ShowPostProcessingSettings() {
        LOG_INFO("========================================");
        LOG_INFO("CURRENT POST-PROCESSING SETTINGS");
        LOG_INFO("========================================");
        
        // Tone mapping
        std::string toneMappingName;
        switch (m_toneMappingType) {
            case ToneMappingType::None: toneMappingName = "None (Linear)"; break;
            case ToneMappingType::Reinhard: toneMappingName = "Reinhard"; break;
            case ToneMappingType::ACES: toneMappingName = "ACES Filmic"; break;
            case ToneMappingType::Filmic: toneMappingName = "Uncharted 2 Filmic"; break;
        }
        LOG_INFO("Tone Mapping: " + toneMappingName);
        LOG_INFO("Exposure: " + std::to_string(m_exposure));
        LOG_INFO("Gamma: " + std::to_string(m_gamma));
        LOG_INFO("");
        
        // FXAA
        LOG_INFO("FXAA Anti-Aliasing: " + std::string(m_fxaaEnabled ? "ENABLED" : "DISABLED"));
        if (m_fxaaEnabled) {
            LOG_INFO("  Quality: " + std::to_string(m_fxaaQuality));
        }
        LOG_INFO("");
        
        // Bloom
        LOG_INFO("Bloom Effect: " + std::string(m_bloomEnabled ? "ENABLED" : "DISABLED"));
        if (m_bloomEnabled) {
            LOG_INFO("  Threshold: " + std::to_string(m_bloomThreshold));
            LOG_INFO("  Intensity: " + std::to_string(m_bloomIntensity));
            LOG_INFO("  Radius: " + std::to_string(m_bloomRadius));
        }
        LOG_INFO("========================================");
    }

    void Render() {
        Math::Mat4 viewProjection = m_camera->GetViewProjectionMatrix();
        m_primitiveRenderer->SetViewProjectionMatrix(viewProjection);

        // Render professional grid
        if (m_gridRenderer) {
            m_gridRenderer->Render(viewProjection);
        }

        // Render ground plane
        m_primitiveRenderer->DrawPlane(Math::Vec3(0.0f, 0.0f, 0.0f),
                                       Math::Vec2(50.0f),
                                       Math::Vec4(0.2f, 0.2f, 0.2f, 1.0f));

        // Render HDR scene objects
        RenderHDRScene();
    }

    void RenderHDRScene() {
        for (const auto& obj : m_sceneObjects) {
            Math::Vec4 renderColor = obj.color;
            
            // Apply emission strength for emissive objects
            if (obj.isEmissive) {
                renderColor.x *= obj.emissionStrength;
                renderColor.y *= obj.emissionStrength;
                renderColor.z *= obj.emissionStrength;
            }
            
            m_primitiveRenderer->DrawCube(obj.position, obj.scale, renderColor);
        }
    }

    void PrintWelcomeMessage() {
        LOG_INFO("========================================");
        LOG_INFO("GAME ENGINE KIRO - POST-PROCESSING EFFECTS DEMONSTRATION");
        LOG_INFO("========================================");
        LOG_INFO("");
        LOG_INFO("POST-PROCESSING FEATURES DEMONSTRATED:");
        LOG_INFO("  ✓ HDR Rendering: High dynamic range scene");
        LOG_INFO("  ✓ Tone Mapping: Multiple tone mapping operators");
        LOG_INFO("  ✓ FXAA Anti-Aliasing: Configurable quality settings");
        LOG_INFO("  ✓ Bloom Effects: HDR bloom with parameter control");
        LOG_INFO("  ✓ Exposure Control: Real-time exposure adjustment");
        LOG_INFO("  ✓ Gamma Correction: Configurable gamma values");
        LOG_INFO("");
        LOG_INFO("NAVIGATION CONTROLS:");
        LOG_INFO("  WASD - Move camera horizontally");
        LOG_INFO("  E/Q - Move camera up/down");
        LOG_INFO("  Mouse - Look around");
        LOG_INFO("  ESC - Toggle mouse capture");
        LOG_INFO("");
        LOG_INFO("POST-PROCESSING CONTROLS:");
        LOG_INFO("  F2 - Cycle tone mapping operators (None/Reinhard/ACES/Filmic)");
        LOG_INFO("  F3 - Toggle FXAA anti-aliasing");
        LOG_INFO("  F4 - Adjust FXAA quality (when enabled)");
        LOG_INFO("  F5 - Toggle bloom effect");
        LOG_INFO("  F6/F7 - Decrease/Increase bloom threshold");
        LOG_INFO("  F8/F9 - Decrease/Increase bloom intensity");
        LOG_INFO("  F10 - Show current post-processing settings");
        LOG_INFO("");
        LOG_INFO("HDR CONTROLS:");
        LOG_INFO("  F11/F12 - Decrease/Increase exposure");
        LOG_INFO("  G - Cycle gamma values");
        LOG_INFO("");
        LOG_INFO("SCENE DESCRIPTION:");
        LOG_INFO("  • 4 bright emissive cubes (HDR values > 1.0)");
        LOG_INFO("  • 3 normal diffuse cubes for comparison");
        LOG_INFO("  • High-intensity lighting for HDR demonstration");
        LOG_INFO("  • Professional grid for reference");
        LOG_INFO("");
        LOG_INFO("  F1 - Exit demonstration");
        LOG_INFO("========================================");
    }

    Engine m_engine;
    std::unique_ptr<PostProcessDemoCamera> m_camera;
    std::unique_ptr<PrimitiveRenderer> m_primitiveRenderer;
    std::unique_ptr<GridRenderer> m_gridRenderer;
    
    std::vector<HDRSceneObject> m_sceneObjects;
    
    // Lighting properties
    Math::Vec3 m_lightDirection;
    Math::Vec3 m_lightColor;
    float m_lightIntensity = 4.0f;
    
    // Post-processing settings
    ToneMappingType m_toneMappingType = ToneMappingType::Reinhard;
    float m_exposure = 1.0f;
    float m_gamma = 2.2f;
    
    // FXAA settings
    bool m_fxaaEnabled = false;
    float m_fxaaQuality = 0.75f;
    
    // Bloom settings
    bool m_bloomEnabled = false;
    float m_bloomThreshold = 1.0f;
    float m_bloomIntensity = 0.5f;
    float m_bloomRadius = 1.0f;
};

int main() {
    PostProcessingDemoApplication app;

    if (!app.Initialize()) {
        LOG_CRITICAL("Failed to initialize post-processing demonstration application");
        return -1;
    }

    app.Run();

    LOG_INFO("Post-processing demonstration terminated successfully");
    return 0;
}