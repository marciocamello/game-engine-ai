#pragma once

#include "GraphicsRenderer.h"
#include <string>
#include <vector>

namespace GameEngine {
    class PostProcessingPipeline;

    class OpenGLRenderer : public GraphicsRenderer {
    public:
        OpenGLRenderer();
        ~OpenGLRenderer() override;

        bool Initialize(const RenderSettings& settings) override;
        void Shutdown() override;
        
        void BeginFrame() override;
        void EndFrame() override;
        void Present() override;
        void Update(float deltaTime) override;

        void SetViewport(int x, int y, int width, int height) override;
        void Clear(const Math::Vec4& color) override;

        void DrawMesh(const Mesh* mesh, const Material* material, const Math::Mat4& transform) override;
        void SetCamera(const Camera* camera) override;

        std::shared_ptr<Shader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource) override;
        std::shared_ptr<Shader> LoadShaderFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) override;
        std::shared_ptr<Shader> GetShader(const std::string& name) override;
        std::shared_ptr<Texture> CreateTexture(const std::string& filepath) override;
        std::shared_ptr<Mesh> CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) override;

        // Enhanced shader management integration
        bool LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, bool enableHotReload = true) override;
        bool UnloadShader(const std::string& name) override;
        void ReloadShader(const std::string& name) override;
        void EnableShaderHotReload(bool enable) override;
        std::vector<std::string> GetLoadedShaderNames() const override;

        // Post-processing pipeline integration
        void EnablePostProcessing(bool enable);
        bool IsPostProcessingEnabled() const { return m_postProcessingEnabled; }
        PostProcessingPipeline* GetPostProcessingPipeline() { return m_postProcessingPipeline.get(); }

        // PBR material support
        void SetupPBRLighting();
        void SetDirectionalLight(const Math::Vec3& direction, const Math::Vec3& color, float intensity = 1.0f);
        void AddPointLight(const Math::Vec3& position, const Math::Vec3& color, float intensity = 1.0f, float radius = 10.0f);
        
        // Camera and lighting information access
        Math::Vec3 GetCameraPosition() const;
        
        // PrimitiveRenderer integration
        void SyncWithPrimitiveRenderer(class PrimitiveRenderer* primitiveRenderer);

    private:
        bool InitializeOpenGL();
        void SetupDebugCallback();
        bool InitializePostProcessing();
        void SetupMainFramebuffer();
        void RenderToMainFramebuffer();
        void ApplyPostProcessing();

        Math::Mat4 m_viewMatrix;
        Math::Mat4 m_projectionMatrix;

        // Post-processing support
        std::unique_ptr<PostProcessingPipeline> m_postProcessingPipeline;
        bool m_postProcessingEnabled = true;
        uint32_t m_mainFramebuffer = 0;
        uint32_t m_mainColorTexture = 0;
        uint32_t m_mainDepthTexture = 0;

        // PBR lighting support
        struct DirectionalLight {
            Math::Vec3 direction = Math::Vec3(0.0f, -1.0f, 0.0f);
            Math::Vec3 color = Math::Vec3(1.0f);
            float intensity = 1.0f;
        } m_directionalLight;

        struct PointLight {
            Math::Vec3 position = Math::Vec3(0.0f);
            Math::Vec3 color = Math::Vec3(1.0f);
            float intensity = 1.0f;
            float radius = 10.0f;
        };
        std::vector<PointLight> m_pointLights;
        static const int MAX_POINT_LIGHTS = 8;
    };
}