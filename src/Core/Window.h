#pragma once

#include "Core/InputState.h"

#include <string>

struct GLFWwindow;

namespace minimap::core {

class Window {
public:
    Window(int width, int height, std::string title, bool visible = true);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    [[nodiscard]] bool ShouldClose() const;
    void PollEvents();
    void SwapBuffers();
    void SetTitle(const std::string& title);

    [[nodiscard]] int Width() const { return width_; }
    [[nodiscard]] int Height() const { return height_; }
    [[nodiscard]] GLFWwindow* Handle() const { return handle_; }
    [[nodiscard]] InputState& Input() { return input_; }
    [[nodiscard]] const InputState& Input() const { return input_; }

private:
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void CursorPosCallback(GLFWwindow* window, double x, double y);
    static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* handle_ {nullptr};
    int width_ {0};
    int height_ {0};
    InputState input_ {};
};

}  // namespace minimap::core
