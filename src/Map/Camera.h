#pragma once

#include "Map/Types.h"

namespace minimap::map {

class Camera {
public:
    void SetCenter(const LatLng& center);
    void SetZoom(float zoom);
    void AddPanPixels(float deltaX, float deltaY, float pixelsPerMeter);
    void AddZoomDelta(float delta);
    void Update(float dtSeconds);

    [[nodiscard]] WorldPoint CenterWorld() const { return centerWorld_; }
    [[nodiscard]] float Zoom() const { return zoomCurrent_; }
    [[nodiscard]] float TargetZoom() const { return zoomTarget_; }
    [[nodiscard]] float ZoomBlend() const { return zoomBlend_; }

private:
    WorldPoint centerWorld_ {};
    float zoomCurrent_ {15.0F};
    float zoomTarget_ {15.0F};
    float zoomBlend_ {0.0F};
};

}  // namespace minimap::map
