#pragma once

#include "Geometry/MeshTypes.h"
#include "GPU/Buffer.h"
#include "GPU/Shader.h"
#include "GPU/Texture2D.h"
#include "GPU/VertexArray.h"
#include "Map/Types.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace minimap::renderer {

struct PoiInstance {
    float x;
    float y;
    float size;
    float headingDeg;
    float alpha;
};

struct RasterQuad {
    minimap::map::TileId id {};
    minimap::map::Aabb bounds {};
    float alpha {1.0F};
    const minimap::map::TileData* tile {nullptr};
};

class TileRenderer {
public:
    TileRenderer();
    void UploadGeometry(const minimap::geometry::Mesh2D& worldMesh, const std::vector<PoiInstance>& instances);
    void UploadRasterQuads(const std::vector<RasterQuad>& rasterQuads);
    void Draw(const minimap::math::Mat4& worldToClip);

private:
    static std::size_t HashTile(const minimap::map::TileId& id);

    gpu::VertexArray worldVao_ {};
    gpu::Buffer worldVbo_ {gpu::BufferType::Vertex};
    gpu::Buffer worldIbo_ {gpu::BufferType::Index};
    gpu::Shader worldShader_ {};
    std::size_t worldIndexCount_ {0};

    gpu::VertexArray poiVao_ {};
    gpu::Buffer poiQuadVbo_ {gpu::BufferType::Vertex};
    gpu::Buffer poiInstanceVbo_ {gpu::BufferType::Vertex};
    gpu::Shader poiShader_ {};
    std::size_t poiInstanceCount_ {0};

    gpu::VertexArray rasterVao_ {};
    gpu::Buffer rasterVbo_ {gpu::BufferType::Vertex};
    gpu::Buffer rasterIbo_ {gpu::BufferType::Index};
    gpu::Shader rasterShader_ {};
    std::vector<RasterQuad> rasterQuads_ {};
    std::unordered_map<std::size_t, std::unique_ptr<gpu::Texture2D>> rasterTextures_ {};
};

}  // namespace minimap::renderer
