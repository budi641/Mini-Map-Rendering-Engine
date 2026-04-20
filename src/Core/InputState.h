#pragma once

#include "Math/Vec2.h"

namespace minimap::core {

struct InputState {
    bool leftMouseDown {false};
    bool rightMouseDown {false};
    math::Vec2 mousePosition {};
    math::Vec2 previousMousePosition {};
    float scrollDelta {0.0F};

    void BeginFrame();
    [[nodiscard]] math::Vec2 MouseDelta() const;
};

}  // namespace minimap::core
