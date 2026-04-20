#include "Map/RoadNetwork.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace minimap::map {

void RoadNetwork::Clear() {
    segments_.clear();
}

void RoadNetwork::AddRoad(const PolylineFeature& road) {
    if (road.points.size() < 2) {
        return;
    }
    for (std::size_t i = 1; i < road.points.size(); ++i) {
        segments_.push_back({road.points[i - 1], road.points[i]});
    }
}

WorldPoint RoadNetwork::SnapToNearest(const WorldPoint& point, double* outDistanceMeters) const {
    double best = std::numeric_limits<double>::max();
    WorldPoint nearest = point;
    for (const auto& seg : segments_) {
        const double abx = seg.b.x - seg.a.x;
        const double aby = seg.b.y - seg.a.y;
        const double apx = point.x - seg.a.x;
        const double apy = point.y - seg.a.y;
        const double len2 = abx * abx + aby * aby;
        if (len2 <= 1e-9) {
            continue;
        }
        const double t = std::clamp((apx * abx + apy * aby) / len2, 0.0, 1.0);
        const WorldPoint projected {seg.a.x + abx * t, seg.a.y + aby * t};
        const double dx = projected.x - point.x;
        const double dy = projected.y - point.y;
        const double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < best) {
            best = dist;
            nearest = projected;
        }
    }
    if (outDistanceMeters != nullptr) {
        *outDistanceMeters = best;
    }
    return nearest;
}

}  // namespace minimap::map
