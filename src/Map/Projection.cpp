#include "Map/Projection.h"

#include <algorithm>
#include <cmath>

namespace minimap::map {

namespace {
constexpr double kPi = 3.14159265358979323846;
}

WorldPoint Projection::LatLngToWorld(const LatLng& latLng) {
    const double latClamped = std::clamp(latLng.lat, -85.05112878, 85.05112878);
    const double x = latLng.lng * kOriginShift / 180.0;
    const double y = std::log(std::tan((90.0 + latClamped) * kPi / 360.0)) * kEarthRadiusMeters;
    return {x, y};
}

LatLng Projection::WorldToLatLng(const WorldPoint& world) {
    const double lng = world.x / kOriginShift * 180.0;
    const double lat = 180.0 / kPi * (2.0 * std::atan(std::exp(world.y / kEarthRadiusMeters)) - kPi / 2.0);
    return {lat, lng};
}

math::Vec2 Projection::WorldToScreen(const WorldPoint& world, const WorldPoint& cameraWorld, float pixelsPerMeter, float viewportWidth, float viewportHeight) {
    const float dx = static_cast<float>(world.x - cameraWorld.x) * pixelsPerMeter;
    const float dy = static_cast<float>(world.y - cameraWorld.y) * pixelsPerMeter;
    return {viewportWidth * 0.5F + dx, viewportHeight * 0.5F - dy};
}

WorldPoint Projection::ScreenToWorld(const math::Vec2& screen, const WorldPoint& cameraWorld, float pixelsPerMeter, float viewportWidth, float viewportHeight) {
    const float dx = screen.x - viewportWidth * 0.5F;
    const float dy = viewportHeight * 0.5F - screen.y;
    return {cameraWorld.x + static_cast<double>(dx / pixelsPerMeter), cameraWorld.y + static_cast<double>(dy / pixelsPerMeter)};
}

}  // namespace minimap::map
