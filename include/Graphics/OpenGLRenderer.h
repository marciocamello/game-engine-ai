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

        void SetViewport(int x, int y, int width, int height) override;
        void Clear(const Math::Vec4& color) override;

        void DrawMesh(const Mesh* mesh, const Material* material, const Math::Mat4& transform) override;
        void SetCamera(const Camera* camera) override;

        std::shared_ptr<Shader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource) override;
        std::shared_ptr<Texture> CreateTexture(const std::string& filepath) override;
        std::shared_ptr<Mesh> CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) override;

    private:
        bool InitializeOpenGL();
        void SetupDebugCallback();

        Math::Mat4 m_viewMatrix;
        Math::Mat4 m_projectionMatrix;
    };
}