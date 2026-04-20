#include "Renderer/TileRenderer.h"

#include <array>
#include <cstdint>
#include <string>

namespace minimap::renderer {

namespace {
std::string ShaderPath(const char* file) {
    return std::string(MINIMAP_SHADER_DIR) + "/" + file;
}
}  // namespace

TileRenderer::TileRenderer() {
    worldShader_.Load(ShaderPath("map.vert"), ShaderPath("map.frag"));
    poiShader_.Load(ShaderPath("poi_instanced.vert"), ShaderPath("poi_instanced.frag"));
    rasterShader_.Load(ShaderPath("raster.vert"), ShaderPath("raster.frag"));

    worldVao_.Bind();
    worldVbo_.Bind();
    worldIbo_.Bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(minimap::geometry::Vertex2D), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(minimap::geometry::Vertex2D), reinterpret_cast<void*>(sizeof(float) * 2));

    poiVao_.Bind();
    constexpr std::array<float, 12> poiQuad = {
        -0.5F, -0.5F, 0.5F, -0.5F, 0.5F, 0.5F,
        -0.5F, -0.5F, 0.5F, 0.5F, -0.5F, 0.5F
    };
    poiQuadVbo_.Upload(poiQuad.data(), sizeof(poiQuad), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, reinterpret_cast<void*>(0));
    poiInstanceVbo_.Bind();
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(PoiInstance), reinterpret_cast<void*>(0));
    glVertexAttribDivisor(1, 1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(PoiInstance), reinterpret_cast<void*>(sizeof(float) * 2));
    glVertexAttribDivisor(2, 1);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(PoiInstance), reinterpret_cast<void*>(sizeof(float) * 3));
    glVertexAttribDivisor(3, 1);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(PoiInstance), reinterpret_cast<void*>(sizeof(float) * 4));
    glVertexAttribDivisor(4, 1);

    rasterVao_.Bind();
    rasterVbo_.Bind();
    rasterIbo_.Bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, reinterpret_cast<void*>(sizeof(float) * 2));

    constexpr std::array<float, 16> quad = {
        0.0F, 0.0F, 0.0F, 1.0F,
        1.0F, 0.0F, 1.0F, 1.0F,
        1.0F, 1.0F, 1.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F
    };
    constexpr std::array<std::uint32_t, 6> idx = {0, 1, 2, 0, 2, 3};
    rasterVbo_.Upload(quad.data(), sizeof(quad), GL_STATIC_DRAW);
    rasterIbo_.Upload(idx.data(), sizeof(idx), GL_STATIC_DRAW);
}

void TileRenderer::UploadGeometry(const minimap::geometry::Mesh2D& worldMesh, const std::vector<PoiInstance>& instances) {
    worldVao_.Bind();
    worldVbo_.Upload(worldMesh.vertices.data(), worldMesh.vertices.size() * sizeof(minimap::geometry::Vertex2D), GL_DYNAMIC_DRAW);
    worldIbo_.Upload(worldMesh.indices.data(), worldMesh.indices.size() * sizeof(std::uint32_t), GL_DYNAMIC_DRAW);
    worldIndexCount_ = worldMesh.indices.size();

    poiVao_.Bind();
    poiInstanceVbo_.Upload(instances.data(), instances.size() * sizeof(PoiInstance), GL_DYNAMIC_DRAW);
    poiInstanceCount_ = instances.size();
}

void TileRenderer::UploadRasterQuads(const std::vector<RasterQuad>& rasterQuads) {
    rasterQuads_ = rasterQuads;
    for (const auto& quad : rasterQuads_) {
        if (quad.tile == nullptr || quad.tile->rasterRgba.empty()) {
            continue;
        }
        const auto key = HashTile(quad.id);
        if (rasterTextures_.find(key) != rasterTextures_.end()) {
            continue;
        }
        auto texture = std::make_unique<gpu::Texture2D>();
        texture->UploadRgba(quad.tile->rasterWidth, quad.tile->rasterHeight, quad.tile->rasterRgba.data());
        rasterTextures_.emplace(key, std::move(texture));
    }
}

void TileRenderer::Draw(const minimap::math::Mat4& worldToClip) {
    worldShader_.Use();
    worldShader_.SetMat4("u_mvp", worldToClip);
    worldVao_.Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(worldIndexCount_), GL_UNSIGNED_INT, nullptr);

    poiShader_.Use();
    poiShader_.SetMat4("u_mvp", worldToClip);
    poiVao_.Bind();
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, static_cast<GLsizei>(poiInstanceCount_));

    if (!rasterQuads_.empty()) {
        rasterShader_.Use();
        rasterShader_.SetMat4("u_mvp", worldToClip);
        rasterShader_.SetInt("u_tex", 0);
        rasterVao_.Bind();
        for (const auto& quad : rasterQuads_) {
            const auto key = HashTile(quad.id);
            const auto it = rasterTextures_.find(key);
            if (it == rasterTextures_.end()) {
                continue;
            }
            rasterShader_.SetVec4("u_bounds", quad.bounds.min.x, quad.bounds.min.y, quad.bounds.max.x, quad.bounds.max.y);
            rasterShader_.SetFloat("u_alpha", quad.alpha);
            it->second->Bind(0);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        }
    }
}

std::size_t TileRenderer::HashTile(const minimap::map::TileId& id) {
    const std::size_t z = static_cast<std::size_t>(id.z) & 0x3F;
    const std::size_t x = static_cast<std::size_t>(id.x) & 0x1FFFFFFF;
    const std::size_t y = static_cast<std::size_t>(id.y) & 0x1FFFFFFF;
    return (z << 58) | (x << 29) | y;
}

}  // namespace minimap::renderer
