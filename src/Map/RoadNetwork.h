#pragma once

#include "Map/Types.h"

#include <vector>

namespace minimap::map {

class RoadNetwork {
public:
    void Clear();
    void AddRoad(const PolylineFeature& road);
    WorldPoint SnapToNearest(const WorldPoint& point, double* outDistanceMeters) const;

private:
    struct Segment {
        WorldPoint a {};
        WorldPoint b {};
    };
    std::vector<Segment> segments_;
};

}  // namespace minimap::map
