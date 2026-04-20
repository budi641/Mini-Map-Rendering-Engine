#pragma once

#include "Math/Vec2.h"

#include <cstdint>
#include <vector>

namespace minimap::geometry {

struct Vertex2D {
    math::Vec2 pos {};
    float r {1.0F};
    float g {1.0F};
    float b {1.0F};
    float a {1.0F};
};

struct Mesh2D {
    std::vector<Vertex2D> vertices;
    std::vector<std::uint32_t> indices;
};

}  // namespace minimap::geometry
