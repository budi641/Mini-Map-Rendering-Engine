#include "Renderer/UiRenderer.h"

#include <array>
#include <cmath>
#include <string>

namespace minimap::renderer {

namespace {
std::string ShaderPath(const char* file) { return std::string(MINIMAP_SHADER_DIR) + "/" + file; }
}  // namespace

UiRenderer::UiRenderer() {
    shader_.Load(ShaderPath("ui.vert"), ShaderPath("ui.frag"));
    vao_.Bind();
    vbo_.Bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, reinterpret_cast<void*>(0));
}

void UiRenderer::DrawCompass(const minimap::math::Mat4& screenToClip, float headingDeg, minimap::math::Vec2 viewportSize) {
    constexpr std::array<float, 6> arrow = {0.0F, -24.0F, 12.0F, 12.0F, -12.0F, 12.0F};
    // Screen space has +Y downward, so positive bearing must rotate clockwise.
    const float rad = headingDeg * 3.14159265F / 180.0F;
    const float c = std::cos(rad);
    const float s = std::sin(rad);
    constexpr float compassInset = 52.0F;
    std::array<float, 6> transformed {};
    for (int i = 0; i < 3; ++i) {
        const float x = arrow[i * 2];
        const float y = arrow[i * 2 + 1];
        transformed[i * 2] = c * x - s * y + viewportSize.x - compassInset;
        transformed[i * 2 + 1] = s * x + c * y + compassInset;
    }
    vbo_.Upload(transformed.data(), sizeof(transformed), GL_DYNAMIC_DRAW);
    shader_.Use();
    shader_.SetMat4("u_mvp", screenToClip);
    shader_.SetVec4("u_color", 0.95F, 0.2F, 0.2F, 0.95F);
    vao_.Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void UiRenderer::DrawMeasurementLine(const minimap::math::Mat4& screenToClip, const minimap::math::Vec2& a, const minimap::math::Vec2& b) {
    const std::array<float, 4> line = {a.x, a.y, b.x, b.y};
    vbo_.Upload(line.data(), sizeof(line), GL_DYNAMIC_DRAW);
    shader_.Use();
    shader_.SetMat4("u_mvp", screenToClip);
    shader_.SetVec4("u_color", 0.15F, 0.65F, 0.95F, 1.0F);
    vao_.Bind();
    glLineWidth(2.0F);
    glDrawArrays(GL_LINES, 0, 2);
}

}  // namespace minimap::renderer
