#include "Map/TileSystem.h"

#include "Map/Projection.h"

#include <algorithm>
#include <cmath>

namespace minimap::map {

double TileSystem::ResolutionMetersPerPixel(double zoom) {
    const double initialResolution = (2.0 * Projection::kOriginShift) / Projection::kTileSize;
    return initialResolution / std::pow(2.0, zoom);
}

double TileSystem::ResolutionMetersPerPixel(int zoom) {
    return ResolutionMetersPerPixel(static_cast<double>(zoom));
}

TileId TileSystem::WorldToTile(const WorldPoint& world, int zoom) {
    const double worldSize = 2.0 * Projection::kOriginShift;
    const double normalizedX = (world.x + Projection::kOriginShift) / worldSize;
    const double normalizedY = (Projection::kOriginShift - world.y) / worldSize;
    const int numTiles = 1 << zoom;
    const int tileX = std::clamp(static_cast<int>(normalizedX * numTiles), 0, numTiles - 1);
    const int tileY = std::clamp(static_cast<int>(normalizedY * numTiles), 0, numTiles - 1);
    return {tileX, tileY, zoom};
}

Aabb TileSystem::TileBoundsWorld(const TileId& tile) {
    const int n = 1 << tile.z;
    const double tileSizeWorld = (2.0 * Projection::kOriginShift) / n;
    const double minX = -Projection::kOriginShift + tile.x * tileSizeWorld;
    const double maxX = minX + tileSizeWorld;
    const double maxY = Projection::kOriginShift - tile.y * tileSizeWorld;
    const double minY = maxY - tileSizeWorld;
    return {{static_cast<float>(minX), static_cast<float>(minY)}, {static_cast<float>(maxX), static_cast<float>(maxY)}};
}

std::vector<TileId> TileSystem::VisibleTiles(const WorldPoint& centerWorld, int zoom, float viewportWidth, float viewportHeight, float pixelsPerMeter) {
    const double halfW = (viewportWidth * 0.5) / pixelsPerMeter;
    const double halfH = (viewportHeight * 0.5) / pixelsPerMeter;
    const WorldPoint topLeft {centerWorld.x - halfW, centerWorld.y + halfH};
    const WorldPoint bottomRight {centerWorld.x + halfW, centerWorld.y - halfH};
    const TileId minTile = WorldToTile(topLeft, zoom);
    const TileId maxTile = WorldToTile(bottomRight, zoom);

    std::vector<TileId> tiles;
    for (int y = minTile.y; y <= maxTile.y; ++y) {
        for (int x = minTile.x; x <= maxTile.x; ++x) {
            tiles.push_back({x, y, zoom});
        }
    }
    return tiles;
}

}  // namespace minimap::map
