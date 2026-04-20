#pragma once

#include "Map/Types.h"

#include <cstdint>
#include <vector>

namespace minimap::geometry {

std::vector<std::uint32_t> TriangulateSimplePolygon(const std::vector<minimap::map::WorldPoint>& polygon);

}  // namespace minimap::geometry
