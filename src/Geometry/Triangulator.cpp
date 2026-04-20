#include "Geometry/Triangulator.h"

#include <algorithm>

namespace minimap::geometry {

namespace {
double SignedArea(const std::vector<minimap::map::WorldPoint>& poly) {
    double area = 0.0;
    for (std::size_t i = 0; i < poly.size(); ++i) {
        const auto& a = poly[i];
        const auto& b = poly[(i + 1) % poly.size()];
        area += a.x * b.y - b.x * a.y;
    }
    return area * 0.5;
}

bool PointInTriangle(const minimap::map::WorldPoint& p, const minimap::map::WorldPoint& a, const minimap::map::WorldPoint& b, const minimap::map::WorldPoint& c) {
    const auto cross = [](const minimap::map::WorldPoint& p1, const minimap::map::WorldPoint& p2, const minimap::map::WorldPoint& p3) {
        return (p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
    };
    const double c1 = cross(a, b, p);
    const double c2 = cross(b, c, p);
    const double c3 = cross(c, a, p);
    const bool neg = (c1 < 0.0) || (c2 < 0.0) || (c3 < 0.0);
    const bool pos = (c1 > 0.0) || (c2 > 0.0) || (c3 > 0.0);
    return !(neg && pos);
}

bool IsEar(int prevIdx, int currIdx, int nextIdx, const std::vector<int>& indices, const std::vector<minimap::map::WorldPoint>& poly, bool ccw) {
    const auto& a = poly[prevIdx];
    const auto& b = poly[currIdx];
    const auto& c = poly[nextIdx];

    const double cross = (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    if (ccw ? (cross <= 0.0) : (cross >= 0.0)) {
        return false;
    }
    for (const int idx : indices) {
        if (idx == prevIdx || idx == currIdx || idx == nextIdx) {
            continue;
        }
        if (PointInTriangle(poly[idx], a, b, c)) {
            return false;
        }
    }
    return true;
}
}  // namespace

std::vector<std::uint32_t> TriangulateSimplePolygon(const std::vector<minimap::map::WorldPoint>& polygon) {
    std::vector<std::uint32_t> triangles;
    if (polygon.size() < 3) {
        return triangles;
    }
    std::vector<int> indices(polygon.size());
    for (int i = 0; i < static_cast<int>(polygon.size()); ++i) {
        indices[i] = i;
    }
    const bool ccw = SignedArea(polygon) > 0.0;
    int guard = 0;
    while (indices.size() > 2 && guard < 10000) {
        bool earFound = false;
        for (std::size_t i = 0; i < indices.size(); ++i) {
            const int prev = indices[(i + indices.size() - 1) % indices.size()];
            const int curr = indices[i];
            const int next = indices[(i + 1) % indices.size()];
            if (!IsEar(prev, curr, next, indices, polygon, ccw)) {
                continue;
            }
            triangles.push_back(static_cast<std::uint32_t>(prev));
            triangles.push_back(static_cast<std::uint32_t>(curr));
            triangles.push_back(static_cast<std::uint32_t>(next));
            indices.erase(indices.begin() + static_cast<std::ptrdiff_t>(i));
            earFound = true;
            break;
        }
        if (!earFound) {
            break;
        }
        ++guard;
    }
    return triangles;
}

}  // namespace minimap::geometry
