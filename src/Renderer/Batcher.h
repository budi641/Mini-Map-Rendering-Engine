#pragma once

#include "Geometry/MeshTypes.h"

namespace minimap::renderer {

class Batcher {
public:
    void Clear();
    void Append(const minimap::geometry::Mesh2D& mesh);
    [[nodiscard]] const minimap::geometry::Mesh2D& Mesh() const { return mesh_; }

private:
    minimap::geometry::Mesh2D mesh_ {};
};

}  // namespace minimap::renderer
