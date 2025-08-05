#pragma once

#include "GraphicsRenderer.h"
#include <string>

namespace GameEngine {
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

    private:
        bool InitializeOpenGL();
        void SetupDebugCallback();

        Math::Mat4 m_viewMatrix;
        Math::Mat4 m_projectionMatrix;
    };
}