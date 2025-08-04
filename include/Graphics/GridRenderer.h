#pragma once

#include "Core/Math.h"
#include <memory>

namespace GameEngine {
    class PrimitiveRenderer;

    class GridRenderer {
    public:
        struct GridSettings {
            float gridSize = 50.0f;
            float gridSpacing = 2.0f;
            Math::Vec4 majorLineColor = Math::Vec4(0.3f, 0.3f, 0.3f, 1.0f);
            Math::Vec4 minorLineColor = Math::Vec4(0.15f, 0.15f, 0.15f, 1.0f);
            float lineWidth = 0.05f;
            int majorLineInterval = 5; // Every 5th line is major
        };

        GridRenderer();
        ~GridRenderer();

        bool Initialize(PrimitiveRenderer* renderer);
        void Render(const Math::Mat4& viewProjection);
        void SetGridSettings(const GridSettings& settings);
        const GridSettings& GetGridSettings() const { return m_settings; }

    private:
        void RenderGridLines();
        
        GridSettings m_settings;
        PrimitiveRenderer* m_primitiveRenderer;
        bool m_initialized;
    };
}