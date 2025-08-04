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
        
        // Render grid lines along Z-axis (running in X direction)
        for (float z = -halfSize; z <= halfSize; z += spacing) {
            int lineIndex = static_cast<int>((z + halfSize) / spacing);
            bool isMajorLine = (lineIndex % m_settings.majorLineInterval) == 0;
            
            Math::Vec4 lineColor = isMajorLine ? m_settings.majorLineColor : m_settings.minorLineColor;
            
            // Create a thin line running from -halfSize to +halfSize in X direction
            Math::Vec3 position(0.0f, lineHeight, z);
            Math::Vec3 scale(halfSize * 2.0f, 0.02f, m_settings.lineWidth);
            
            m_primitiveRenderer->DrawCube(position, scale, lineColor);
        }
        
        // Render grid lines along X-axis (running in Z direction)
        for (float x = -halfSize; x <= halfSize; x += spacing) {
            int lineIndex = static_cast<int>((x + halfSize) / spacing);
            bool isMajorLine = (lineIndex % m_settings.majorLineInterval) == 0;
            
            Math::Vec4 lineColor = isMajorLine ? m_settings.majorLineColor : m_settings.minorLineColor;
            
            // Create a thin line running from -halfSize to +halfSize in Z direction
            Math::Vec3 position(x, lineHeight, 0.0f);
            Math::Vec3 scale(m_settings.lineWidth, 0.02f, halfSize * 2.0f);
            
            m_primitiveRenderer->DrawCube(position, scale, lineColor);
        }
    }
}