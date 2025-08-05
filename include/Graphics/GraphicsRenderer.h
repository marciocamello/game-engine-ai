#pragma once

#include "Core/Math.h"
#include <memory>
#include <vector>
#include <string>

struct GLFWwindow;

namespace GameEngine {
    enum class GraphicsAPI {
        OpenGL,
        Vulkan
    };

    struct RenderSettings {
        int windowWidth = 1920;
        int windowHeight = 1080;
        bool fullscreen = false;
        bool vsync = true;
        int msaaSamples = 4;
        GraphicsAPI api = GraphicsAPI::OpenGL;
    };

    class Camera;
    class Mesh;
    class Material;
    class Texture;
    class Shader;

    class GraphicsRenderer {
    public:
        GraphicsRenderer();
        virtual ~GraphicsRenderer();

        virtual bool Initialize(const RenderSettings& settings) = 0;
        virtual void Shutdown() = 0;
        
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Present() = 0;
        virtual void Update(float deltaTime) = 0;

        virtual void SetViewport(int x, int y, int width, int height) = 0;
        virtual void Clear(const Math::Vec4& color = Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f)) = 0;

        // Rendering commands
        virtual void DrawMesh(const Mesh* mesh, const Material* material, const Math::Mat4& transform) = 0;
        virtual void SetCamera(const Camera* camera) = 0;

        // Resource creation
        virtual std::shared_ptr<Shader> CreateShader(const std::string& vertexSource, const std::string& fragmentSource) = 0;
        virtual std::shared_ptr<Shader> LoadShaderFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) = 0;
        virtual std::shared_ptr<Shader> GetShader(const std::string& name) = 0;
        virtual std::shared_ptr<Texture> CreateTexture(const std::string& filepath) = 0;
        virtual std::shared_ptr<Mesh> CreateMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) = 0;

        // Enhanced shader management integration
        virtual bool LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath, bool enableHotReload = true) = 0;
        virtual bool UnloadShader(const std::string& name) = 0;
        virtual void ReloadShader(const std::string& name) = 0;
        virtual void EnableShaderHotReload(bool enable) = 0;
        virtual std::vector<std::string> GetLoadedShaderNames() const = 0;

        GLFWwindow* GetWindow() const { return m_window; }
        const RenderSettings& GetSettings() const { return m_settings; }

        static std::unique_ptr<GraphicsRenderer> Create(GraphicsAPI api);

    protected:
        GLFWwindow* m_window = nullptr;
        RenderSettings m_settings;
    };
}