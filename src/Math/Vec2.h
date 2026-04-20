#pragma once

#include <cmath>

namespace minimap::math {

struct Vec2 {
    float x {0.0F};
    float y {0.0F};

    Vec2() = default;
    Vec2(float xIn, float yIn) : x(xIn), y(yIn) {}

    Vec2 operator+(const Vec2& rhs) const { return {x + rhs.x, y + rhs.y}; }
    Vec2 operator-(const Vec2& rhs) const { return {x - rhs.x, y - rhs.y}; }
    Vec2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
    Vec2 operator/(float scalar) const { return {x / scalar, y / scalar}; }
    Vec2& operator+=(const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    float length() const { return std::sqrt(x * x + y * y); }
    float lengthSquared() const { return x * x + y * y; }
};

inline float Dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
inline float Cross(const Vec2& a, const Vec2& b) { return a.x * b.y - a.y * b.x; }
inline Vec2 Normalize(const Vec2& v) {
    const float len = v.length();
    if (len <= 1e-6F) {
        return {};
    }
    return v / len;
}

}  // namespace minimap::math
