#include "Math/Mat4.h"

#include <cmath>

namespace minimap::math {

Mat4::Mat4() : values_ {1.0F, 0.0F, 0.0F, 0.0F,
                        0.0F, 1.0F, 0.0F, 0.0F,
                        0.0F, 0.0F, 1.0F, 0.0F,
                        0.0F, 0.0F, 0.0F, 1.0F} {}

Mat4 Mat4::Identity() { return Mat4(); }

Mat4 Mat4::Ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    Mat4 out {};
    out.values_ = {
        2.0F / (right - left), 0.0F, 0.0F, 0.0F,
        0.0F, 2.0F / (top - bottom), 0.0F, 0.0F,
        0.0F, 0.0F, -2.0F / (farPlane - nearPlane), 0.0F,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(farPlane + nearPlane) / (farPlane - nearPlane), 1.0F
    };
    return out;
}

Mat4 Mat4::Translate(float x, float y, float z) {
    Mat4 out = Identity();
    out.values_[12] = x;
    out.values_[13] = y;
    out.values_[14] = z;
    return out;
}

Mat4 Mat4::RotateZ(float radians) {
    Mat4 out = Identity();
    const float c = std::cos(radians);
    const float s = std::sin(radians);
    out.values_[0] = c;
    out.values_[1] = s;
    out.values_[4] = -s;
    out.values_[5] = c;
    return out;
}

Mat4 Mat4::Scale(float x, float y, float z) {
    Mat4 out = Identity();
    out.values_[0] = x;
    out.values_[5] = y;
    out.values_[10] = z;
    return out;
}

Mat4 Mat4::operator*(const Mat4& rhs) const {
    Mat4 result;
    result.values_.fill(0.0F);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            for (int k = 0; k < 4; ++k) {
                result.values_[row + col * 4] += values_[row + k * 4] * rhs.values_[k + col * 4];
            }
        }
    }
    return result;
}

const float* Mat4::data() const { return values_.data(); }

}  // namespace minimap::math
