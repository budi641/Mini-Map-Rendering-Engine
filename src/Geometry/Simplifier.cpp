#include "Geometry/Simplifier.h"

#include <cmath>

namespace minimap::geometry {

namespace {
double PerpDistance(const minimap::map::WorldPoint& p, const minimap::map::WorldPoint& a, const minimap::map::WorldPoint& b) {
    const double abx = b.x - a.x;
    const double aby = b.y - a.y;
    const double len2 = abx * abx + aby * aby;
    if (len2 < 1e-9) {
        const double dx = p.x - a.x;
        const double dy = p.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    const double t = ((p.x - a.x) * abx + (p.y - a.y) * aby) / len2;
    const double projX = a.x + abx * t;
    const double projY = a.y + aby * t;
    const double dx = p.x - projX;
    const double dy = p.y - projY;
    return std::sqrt(dx * dx + dy * dy);
}

void SimplifyRecursive(const std::vector<minimap::map::WorldPoint>& input, int first, int last, double eps, std::vector<bool>* keep) {
    double maxDist = 0.0;
    int index = -1;
    for (int i = first + 1; i < last; ++i) {
        const double dist = PerpDistance(input[i], input[first], input[last]);
        if (dist > maxDist) {
            maxDist = dist;
            index = i;
        }
    }
    if (index >= 0 && maxDist > eps) {
        (*keep)[index] = true;
        SimplifyRecursive(input, first, index, eps, keep);
        SimplifyRecursive(input, index, last, eps, keep);
    }
}
}  // namespace

std::vector<minimap::map::WorldPoint> DouglasPeucker(const std::vector<minimap::map::WorldPoint>& input, double epsilonMeters) {
    if (input.size() <= 2) {
        return input;
    }
    std::vector<bool> keep(input.size(), false);
    keep.front() = true;
    keep.back() = true;
    SimplifyRecursive(input, 0, static_cast<int>(input.size() - 1), epsilonMeters, &keep);

    std::vector<minimap::map::WorldPoint> output;
    output.reserve(input.size());
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (keep[i]) {
            output.push_back(input[i]);
        }
    }
    return output;
}

}  // namespace minimap::geometry
