#pragma once

#include "Core/Window.h"
#include "Renderer/RenderEngine.h"

namespace minimap::core {

class Application {
public:
    Application();
    void Run();

private:
    Window window_;
    renderer::RenderEngine engine_;
};

}  // namespace minimap::core
