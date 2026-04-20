#include "Map/Camera.h"

#include "Map/Projection.h"

#include <algorithm>

namespace minimap::map {

void Camera::SetCenter(const LatLng& center) {
    centerWorld_ = Projection::LatLngToWorld(center);
}

void Camera::SetZoom(float zoom) {
    zoomCurrent_ = std::clamp(zoom, 2.0F, 20.0F);
    zoomTarget_ = zoomCurrent_;
}

void Camera::AddPanPixels(float deltaX, float deltaY, float pixelsPerMeter) {
    centerWorld_.x -= deltaX / pixelsPerMeter;
    centerWorld_.y += deltaY / pixelsPerMeter;
}

void Camera::AddZoomDelta(float delta) {
    zoomTarget_ = std::clamp(zoomTarget_ + delta, 2.0F, 20.0F);
}

void Camera::Update(float dtSeconds) {
    const float smooth = 1.0F - std::exp(-12.0F * dtSeconds);
    const float prev = zoomCurrent_;
    zoomCurrent_ = zoomCurrent_ + (zoomTarget_ - zoomCurrent_) * smooth;
    zoomBlend_ = std::clamp(std::abs(zoomCurrent_ - prev) * 4.0F, 0.0F, 1.0F);
}

}  // namespace minimap::map
