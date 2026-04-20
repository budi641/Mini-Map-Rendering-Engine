#pragma once

#include "Geometry/MeshTypes.h"
#include "Math/Vec2.h"

#include <vector>

namespace minimap::geometry {

Mesh2D BuildPolylineMesh(const std::vector<math::Vec2>& points, float thicknessPixels, float r, float g, float b);

}  // namespace minimap::geometry
