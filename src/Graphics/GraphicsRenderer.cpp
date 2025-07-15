#include "Graphics/GraphicsRenderer.h"
#include "Graphics/OpenGLRenderer.h"
#include "Core/Logger.h"

namespace GameEngine {
    GraphicsRenderer::GraphicsRenderer() {
    }

    GraphicsRenderer::~GraphicsRenderer() {
    }

    std::unique_ptr<GraphicsRenderer> GraphicsRenderer::Create(GraphicsAPI api) {
        switch (api) {
            case GraphicsAPI::OpenGL:
                return std::make_unique<OpenGLRenderer>();
            case GraphicsAPI::Vulkan:
                LOG_WARNING("Vulkan renderer not yet implemented, falling back to OpenGL");
                return std::make_unique<OpenGLRenderer>();
            default:
                LOG_ERROR("Unknown graphics API specified");
                return nullptr;
        }
    }
}