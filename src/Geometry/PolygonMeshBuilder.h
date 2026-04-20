#pragma once

#include "Geometry/MeshTypes.h"
#include "Map/Types.h"

namespace minimap::geometry {

Mesh2D BuildPolygonMesh(const minimap::map::PolygonFeature& feature);

}  // namespace minimap::geometry
