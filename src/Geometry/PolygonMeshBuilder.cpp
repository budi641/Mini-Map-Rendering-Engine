#include "Geometry/PolygonMeshBuilder.h"

#include "Geometry/Triangulator.h"

namespace minimap::geometry {

Mesh2D BuildPolygonMesh(const minimap::map::PolygonFeature& feature) {
    Mesh2D mesh;
    for (const auto& p : feature.outer) {
        mesh.vertices.push_back({
            {static_cast<float>(p.x), static_cast<float>(p.y)},
            feature.colorRg.x,
            feature.colorRg.y,
            feature.colorB,
            1.0F
        });
    }
    mesh.indices = TriangulateSimplePolygon(feature.outer);
    return mesh;
}

}  // namespace minimap::geometry
