#pragma once

#include <array>

namespace minimap::math {

class Mat4 {
public:
    Mat4();

    static Mat4 Identity();
    static Mat4 Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static Mat4 Translate(float x, float y, float z);
    static Mat4 RotateZ(float radians);
    static Mat4 Scale(float x, float y, float z);

    Mat4 operator*(const Mat4& rhs) const;
    const float* data() const;

private:
    std::array<float, 16> values_;
};

}  // namespace minimap::math
