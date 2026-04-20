#pragma once

#include "Map/Types.h"

#include <vector>

namespace minimap::map {

class TileSystem {
public:
    static double ResolutionMetersPerPixel(double zoom);
    static double ResolutionMetersPerPixel(int zoom);
    static TileId WorldToTile(const WorldPoint& world, int zoom);
    static Aabb TileBoundsWorld(const TileId& tile);
    static std::vector<TileId> VisibleTiles(const WorldPoint& centerWorld, int zoom, float viewportWidth, float viewportHeight, float pixelsPerMeter);
};

}  // namespace minimap::map
