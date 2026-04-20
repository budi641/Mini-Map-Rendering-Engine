#include "Map/Distance.h"

#include "Map/Projection.h"

#include <cmath>

namespace minimap::map {

namespace {
constexpr double kPi = 3.14159265358979323846;
double ToRad(double deg) { return deg * kPi / 180.0; }
double ToDeg(double rad) { return rad * 180.0 / kPi; }
}  // namespace

double HaversineMeters(const LatLng& a, const LatLng& b) {
    const double dLat = ToRad(b.lat - a.lat);
    const double dLng = ToRad(b.lng - a.lng);
    const double lat1 = ToRad(a.lat);
    const double lat2 = ToRad(b.lat);
    const double sinLat = std::sin(dLat * 0.5);
    const double sinLng = std::sin(dLng * 0.5);
    const double h = sinLat * sinLat + std::cos(lat1) * std::cos(lat2) * sinLng * sinLng;
    return 2.0 * Projection::kEarthRadiusMeters * std::asin(std::sqrt(h));
}

double BearingDegrees(const LatLng& from, const LatLng& to) {
    const double lat1 = ToRad(from.lat);
    const double lat2 = ToRad(to.lat);
    const double dLng = ToRad(to.lng - from.lng);
    const double y = std::sin(dLng) * std::cos(lat2);
    const double x = std::cos(lat1) * std::sin(lat2) - std::sin(lat1) * std::cos(lat2) * std::cos(dLng);
    const double bearing = ToDeg(std::atan2(y, x));
    return std::fmod(bearing + 360.0, 360.0);
}

}  // namespace minimap::map
