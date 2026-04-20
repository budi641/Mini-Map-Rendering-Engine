#pragma once

#include "Map/Types.h"

namespace minimap::map {

double HaversineMeters(const LatLng& a, const LatLng& b);
double BearingDegrees(const LatLng& from, const LatLng& to);

}  // namespace minimap::map
