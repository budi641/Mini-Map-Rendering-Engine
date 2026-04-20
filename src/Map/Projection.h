#pragma once

#include "Map/Types.h"
#include "Math/Vec2.h"

namespace minimap::map {

class Projection {
public:
    static constexpr double kEarthRadiusMeters = 6378137.0;
    static constexpr double kOriginShift = 20037508.342789244;
    static constexpr int kTileSize = 256;

    static WorldPoint LatLngToWorld(const LatLng& latLng);
    static LatLng WorldToLatLng(const WorldPoint& world);
    static math::Vec2 WorldToScreen(const WorldPoint& world, const WorldPoint& cameraWorld, float pixelsPerMeter, float viewportWidth, float viewportHeight);
    static WorldPoint ScreenToWorld(const math::Vec2& screen, const WorldPoint& cameraWorld, float pixelsPerMeter, float viewportWidth, float viewportHeight);
};

}  // namespace minimap::map
