#include "Graphics/GridRenderer.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Core/Logger.h"

namespace GameEngine {
    GridRenderer::GridRenderer() 
        : m_primitiveRenderer(nullptr), m_initialized(false) {
    }

    GridRenderer::~GridRenderer() {
    }

    bool GridRenderer::Initialize(PrimitiveRenderer* renderer) {
        if (!renderer) {
            LOG_ERROR("GridRenderer: PrimitiveRenderer is null");
            return false;
        }

        m_primitiveRenderer = renderer;
        m_initialized = true;

        LOG_INFO("GridRenderer initialized successfully");
        return true;
    }

    void GridRenderer::Render(const Math::Mat4& viewProjection) {
        if (!m_initialized || !m_primitiveRenderer) {
            return;
        }

        m_primitiveRenderer->SetViewProjectionMatrix(viewProjection);
        RenderGridLines();
    }

    void GridRenderer::SetGridSettings(const GridSettings& settings) {
        m_settings = settings;
    }

    void GridRenderer::RenderGridLines() {
        const float halfSize = m_settings.gridSize;
        const float spacing = m_settings.gridSpacing;
        const float lineHeight = 0.01f; // Slightly above ground plane
        
        // Optimize by reducing grid density for better performance
        const float optimizedSpacing = std::max(spacing, 1.0f); // Minimum 1 unit spacing
        const float optimizedSize = halfSize;  // Use full grid size to match ground collision
        
        // Batch render minor lines first
        for (float z = -optimizedSize; z <= optimizedSize; z += optimizedSpacing) {
            int lineIndex = static_cast<int>((z + optimizedSize) / optimizedSpacing);
            if ((lineIndex % m_settings.majorLineInterval) != 0) { // Only minor lines
                Math::Vec3 position(0.0f, lineHeight, z);
                Math::Vec3 scale(optimizedSize * 2.0f, 0.02f, m_settings.lineWidth);
                m_primitiveRenderer->DrawCube(position, scale, m_settings.minorLineColor);
            }
        }
        
        for (float x = -optimizedSize; x <= optimizedSize; x += optimizedSpacing) {
            int lineIndex = static_cast<int>((x + optimizedSize) / optimizedSpacing);
            if ((lineIndex % m_settings.majorLineInterval) != 0) { // Only minor lines
                Math::Vec3 position(x, lineHeight, 0.0f);
                Math::Vec3 scale(m_settings.lineWidth, 0.02f, optimizedSize * 2.0f);
                m_primitiveRenderer->DrawCube(position, scale, m_settings.minorLineColor);
            }
        }
        
        // Then render major lines (fewer of them, more visible)
        for (float z = -optimizedSize; z <= optimizedSize; z += optimizedSpacing) {
            int lineIndex = static_cast<int>((z + optimizedSize) / optimizedSpacing);
            if ((lineIndex % m_settings.majorLineInterval) == 0) { // Only major lines
                Math::Vec3 position(0.0f, lineHeight, z);
                Math::Vec3 scale(optimizedSize * 2.0f, 0.02f, m_settings.lineWidth * 1.5f); // Slightly thicker
                m_primitiveRenderer->DrawCube(position, scale, m_settings.majorLineColor);
            }
        }
        
        for (float x = -optimizedSize; x <= optimizedSize; x += optimizedSpacing) {
            int lineIndex = static_cast<int>((x + optimizedSize) / optimizedSpacing);
            if ((lineIndex % m_settings.majorLineInterval) == 0) { // Only major lines
                Math::Vec3 position(x, lineHeight, 0.0f);
                Math::Vec3 scale(m_settings.lineWidth * 1.5f, 0.02f, optimizedSize * 2.0f); // Slightly thicker
                m_primitiveRenderer->DrawCube(position, scale, m_settings.majorLineColor);
            }
        }
    }
}