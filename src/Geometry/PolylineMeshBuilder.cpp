#include "Geometry/PolylineMeshBuilder.h"

namespace minimap::geometry {

Mesh2D BuildPolylineMesh(const std::vector<math::Vec2>& points, float thicknessPixels, float r, float g, float b) {
    Mesh2D mesh;
    if (points.size() < 2) {
        return mesh;
    }
    const float half = thicknessPixels * 0.5F;
    for (std::size_t i = 1; i < points.size(); ++i) {
        const math::Vec2 p0 = points[i - 1];
        const math::Vec2 p1 = points[i];
        const math::Vec2 dir = math::Normalize(p1 - p0);
        const math::Vec2 n {-dir.y, dir.x};
        const math::Vec2 v0 = p0 + n * half;
        const math::Vec2 v1 = p0 - n * half;
        const math::Vec2 v2 = p1 + n * half;
        const math::Vec2 v3 = p1 - n * half;

        const std::uint32_t base = static_cast<std::uint32_t>(mesh.vertices.size());
        mesh.vertices.push_back({v0, r, g, b, 1.0F});
        mesh.vertices.push_back({v1, r, g, b, 1.0F});
        mesh.vertices.push_back({v2, r, g, b, 1.0F});
        mesh.vertices.push_back({v3, r, g, b, 1.0F});

        mesh.indices.insert(mesh.indices.end(), {
            base + 0, base + 1, base + 2,
            base + 2, base + 1, base + 3
        });
    }
    return mesh;
}

}  // namespace minimap::geometry
