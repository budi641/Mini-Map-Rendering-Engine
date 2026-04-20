#pragma once

#include "GPU/Buffer.h"
#include "GPU/Shader.h"
#include "GPU/VertexArray.h"
#include "Math/Mat4.h"
#include "Math/Vec2.h"

namespace minimap::renderer {

class UiRenderer {
public:
    UiRenderer();
    void DrawCompass(const minimap::math::Mat4& screenToClip, float headingDeg, minimap::math::Vec2 viewportSize);
    void DrawMeasurementLine(const minimap::math::Mat4& screenToClip, const minimap::math::Vec2& a, const minimap::math::Vec2& b);

private:
    gpu::VertexArray vao_ {};
    gpu::Buffer vbo_ {gpu::BufferType::Vertex};
    gpu::Shader shader_ {};
};

}  // namespace minimap::renderer
