#pragma once

#include "Math/Vec2.h"

#include <cstdint>
#include <vector>

namespace minimap::map {

struct LatLng {
    double lat {0.0};
    double lng {0.0};
};

struct WorldPoint {
    double x {0.0};
    double y {0.0};
};

struct TileId {
    int x {0};
    int y {0};
    int z {0};
};

struct Aabb {
    math::Vec2 min {};
    math::Vec2 max {};
};

struct PolylineFeature {
    std::vector<WorldPoint> points;
    float widthPixels {2.0F};
    math::Vec2 colorRg {};
    float colorB {0.0F};
};

struct PolygonFeature {
    std::vector<WorldPoint> outer;
    math::Vec2 colorRg {};
    float colorB {0.0F};
};

struct PoiFeature {
    WorldPoint position {};
    float sizePixels {6.0F};
    float headingDegrees {0.0F};
};

struct TileData {
    TileId id {};
    std::vector<PolylineFeature> roads;
    std::vector<PolygonFeature> landuse;
    std::vector<PoiFeature> pois;
    std::vector<unsigned char> rasterRgba;
    unsigned int rasterWidth {0};
    unsigned int rasterHeight {0};
};

}  // namespace minimap::map
