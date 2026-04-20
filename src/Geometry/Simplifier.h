#pragma once

#include "Map/Types.h"

#include <vector>

namespace minimap::geometry {

std::vector<minimap::map::WorldPoint> DouglasPeucker(const std::vector<minimap::map::WorldPoint>& input, double epsilonMeters);

}  // namespace minimap::geometry
