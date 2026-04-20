#include "Core/InputState.h"

namespace minimap::core {

void InputState::BeginFrame() {
    previousMousePosition = mousePosition;
    scrollDelta = 0.0F;
}

math::Vec2 InputState::MouseDelta() const {
    return mousePosition - previousMousePosition;
}

}  // namespace minimap::core
