#include "Core/Application.h"

#include "GPU/GlHeaders.h"
#include "Map/Projection.h"

#include <sstream>

namespace minimap::core {

Application::Application() : window_(1280, 720, "Mini Map Engine"), engine_() {
    engine_.Initialize(window_.Width(), window_.Height());
}

void Application::Run() {
    double last = glfwGetTime();
    while (!window_.ShouldClose()) {
        window_.PollEvents();
        const double now = glfwGetTime();
        const float dt = static_cast<float>(now - last);
        last = now;

        engine_.OnResize(window_.Width(), window_.Height());
        engine_.HandleInput(window_.Input(), dt);
        engine_.Update(dt);
        engine_.Render();

        if (engine_.HasDistanceMeasurement()) {
            std::ostringstream title;
            title << "Mini Map Engine | Distance: " << engine_.DistanceMeters() << "m | Bearing: " << engine_.BearingDegrees() << "deg";
            window_.SetTitle(title.str());
        } else {
            window_.SetTitle("Mini Map Engine");
        }

        window_.SwapBuffers();
    }
}

}  // namespace minimap::core
