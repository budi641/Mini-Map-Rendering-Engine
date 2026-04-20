#include "Core/Window.h"

#include "GPU/GlHeaders.h"

#include <stdexcept>

namespace minimap::core {

Window::Window(int width, int height, std::string title, bool visible) : width_(width), height_(height) {
    if (glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);

    handle_ = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (handle_ == nullptr) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(handle_);
    glfwSetWindowUserPointer(handle_, this);
    glfwSetMouseButtonCallback(handle_, MouseButtonCallback);
    glfwSetCursorPosCallback(handle_, CursorPosCallback);
    glfwSetScrollCallback(handle_, ScrollCallback);
    glfwSetFramebufferSizeCallback(handle_, FramebufferSizeCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW");
    }

    glViewport(0, 0, width_, height_);
}

Window::~Window() {
    if (handle_ != nullptr) {
        glfwDestroyWindow(handle_);
    }
    glfwTerminate();
}

bool Window::ShouldClose() const { return glfwWindowShouldClose(handle_) != 0; }

void Window::PollEvents() {
    input_.BeginFrame();
    glfwPollEvents();
}

void Window::SwapBuffers() { glfwSwapBuffers(handle_); }

void Window::SetTitle(const std::string& title) {
    glfwSetWindowTitle(handle_, title.c_str());
}

void Window::MouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        self->input_.leftMouseDown = (action == GLFW_PRESS);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        self->input_.rightMouseDown = (action == GLFW_PRESS);
    }
}

void Window::CursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->input_.mousePosition = {static_cast<float>(x), static_cast<float>(y)};
}

void Window::ScrollCallback(GLFWwindow* window, double, double yOffset) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->input_.scrollDelta += static_cast<float>(yOffset);
}

void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->width_ = width;
    self->height_ = height;
    glViewport(0, 0, width, height);
}

}  // namespace minimap::core
