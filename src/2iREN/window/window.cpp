#include "window.hpp"

#include <GLFW/glfw3.h>

#include "2iREN/core/assert.hpp"
#include "2iREN/math/bounded.hpp"
#include "2iREN/math/extent.hpp"
#include "2iREN/math/point2.hpp"
#include "2iREN/utility/log.hpp"
#include "2iREN/window/mappings.hpp"

namespace siren {

Window::Window(const WindowDescriptor& descriptor) : m_input(*this) {
    GLFWmonitor* monitor = nullptr;

    if (descriptor.mode == WindowMode::Fullscreen) {
        monitor = glfwGetPrimaryMonitor();
    }

    glfwWindowHint(GLFW_DECORATED, descriptor.decorated);
    glfwWindowHint(GLFW_RESIZABLE, descriptor.resizable);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, descriptor.transparent);

    m_handle = glfwCreateWindow(
        descriptor.width, descriptor.height, descriptor.title.c_str(), monitor, nullptr
    );

    ASSERT(m_handle, "Failed to create GLFW window");

    // setup callbacks
    glfwSetWindowUserPointer(m_handle, this);
    glfwSetKeyCallback(m_handle, glfw_key_callback);
    glfwSetMouseButtonCallback(m_handle, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(m_handle, glfw_mouse_move_callback);
    glfwSetScrollCallback(m_handle, glfw_scroll_callback);
    glfwSetFramebufferSizeCallback(m_handle, glfw_framebuffer_resize_callback);

    f64 mouse_x = 0.0;
    f64 mouse_y = 0.0;
    glfwGetCursorPos(m_handle, &mouse_x, &mouse_y);
    m_input.initialize_mouse_position(
        Point2f{static_cast<f32>(mouse_x), static_cast<f32>(mouse_y)}
    );

    log::info("window created");

    glfwDefaultWindowHints();
    glfwMakeContextCurrent(m_handle);
}

Window::~Window() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
}

auto Window::aspect() const noexcept -> NonZeroPositiveF32 {
    const auto ex = extent();
    return static_cast<f32>(ex.x) / static_cast<f32>(ex.y);
}

auto Window::extent() const noexcept -> Extent2u {
    i32 x, y;
    glfwGetWindowSize(m_handle, &x, &y);
    ASSERT(x > 0 && y > 0, "glfw error: window extent cannot be negative");

    return Extent2u{
        static_cast<u32>(x),
        static_cast<u32>(y),
    };
}

auto Window::framebuffer_extent() const noexcept -> Extent2u {
    i32 x, y;
    glfwGetFramebufferSize(m_handle, &x, &y);
    ASSERT(x > 0 && y > 0, "glfw error: window framebuffer extent cannot be negative");

    return Extent2u{
        static_cast<u32>(x),
        static_cast<u32>(y),
    };
}

auto Window::position() const noexcept -> Point2u {
    i32 x, y;
    glfwGetWindowPos(m_handle, &x, &y);
    ASSERT(x > 0 && y > 0, "glfw error: window position cannot be negative");

    return Point2u{
        static_cast<u32>(x),
        static_cast<u32>(y),
    };
}

auto Window::title() const noexcept -> std::string {
    return glfwGetWindowTitle(m_handle);
}

auto Window::mode() const noexcept -> WindowMode {
    if (glfwGetWindowMonitor(m_handle) != nullptr) {
        return WindowMode::Fullscreen;
    }

    if (glfwGetWindowAttrib(m_handle, GLFW_ICONIFIED)) {
        return WindowMode::Minimized;
    };

    if (glfwGetWindowAttrib(m_handle, GLFW_MAXIMIZED)) {
        return WindowMode::Maximized;
    };

    return WindowMode::Normal;
}

auto Window::cursor_mode() const noexcept -> CursorMode {
    return from_glfw_mouse_mode(static_cast<u32>(glfwGetInputMode(m_handle, GLFW_CURSOR)));
}

auto Window::should_close() const noexcept -> bool {
    return m_handle == nullptr || glfwWindowShouldClose(m_handle);
}

auto Window::native_handle() const noexcept -> GLFWwindow* {
    return m_handle;
}

auto Window::input() const noexcept -> const Input& {
    return m_input;
}

auto Window::input() noexcept -> Input& {
    return m_input;
}

auto Window::set_title(const std::string& title) -> void {
    glfwSetWindowTitle(m_handle, title.c_str());
    log::trace("Window title set to {}", title);
}

auto Window::set_mode(const WindowMode mode) -> void {
    switch (mode) {
        case WindowMode::Normal: {
            log::trace("window set to normal");
            glfwSetWindowMonitor(m_handle, nullptr, 0, 0, 1280, 720, 0);
            break;
        }
        case WindowMode::Minimized: {
            log::trace("window minimized");
            glfwIconifyWindow(m_handle);
            break;
        }
        case WindowMode::Maximized: {
            log::trace("window maximized");
            glfwMaximizeWindow(m_handle);
            break;
        }
        case WindowMode::Fullscreen: {
            log::trace("window set to fullscreen");
            const auto monitor = glfwGetPrimaryMonitor();
            ASSERT(monitor, "glfw failed to get the primary monitor.");

            const auto videomode = glfwGetVideoMode(monitor);
            ASSERT(videomode, "glfw failed to get the videomode of the primary monitor.");

            glfwSetWindowMonitor(
                m_handle, monitor, 0, 0, videomode->width, videomode->height, videomode->refreshRate
            );
            break;
        }
    }
}

auto Window::set_extent(const Extent2u extent) -> void {
    glfwSetWindowSize(m_handle, static_cast<i32>(extent.x), static_cast<i32>(extent.y));
    log::trace("window extent set to {}.", extent);
}

auto Window::set_position(Point2u position) -> void {
    glfwSetWindowPos(m_handle, position.x, position.y);
    log::trace("window position set to ({}, {})", position.x, position.y);
}

auto Window::set_cursor_mode(const CursorMode cursormode) -> void {
    glfwSetInputMode(m_handle, GLFW_CURSOR, static_cast<i32>(to_glfw(cursormode)));
}

auto Window::poll_events() -> void {
    m_input.update();
    glfwPollEvents();
}

auto Window::on_resize(OnResizeCallback&& callback) -> void {
    m_resize_callback = std::move(callback);
}

void Window::glfw_key_callback(
    GLFWwindow* window,
    const i32 key,
    const i32,
    const i32 action,
    const i32
) {
    if (key == GLFW_KEY_UNKNOWN) {
        return;
    }

    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        self->m_input.press(from_glfw_key(static_cast<u32>(key)));
    } else if (action == GLFW_RELEASE) {
        self->m_input.release(from_glfw_key(static_cast<u32>(key)));
    }
}

auto Window::glfw_mouse_button_callback(
    GLFWwindow* window,
    const i32 button,
    const i32 action,
    const i32
) -> void {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        self->m_input.press(from_glfw_mouse(static_cast<u32>(button)));
    } else if (action == GLFW_RELEASE) {
        self->m_input.release(from_glfw_mouse(static_cast<u32>(button)));
    }
}

auto Window::glfw_mouse_move_callback(GLFWwindow* window, const f64 xpos, const f64 ypos) -> void {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->m_input.move_mouse(Point2f{static_cast<f32>(xpos), static_cast<f32>(ypos)});
}

auto Window::glfw_scroll_callback(GLFWwindow* window, const f64 xoffset, const f64 yoffset)
    -> void {
    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->m_input.scroll_mouse(Vec2f{static_cast<f32>(xoffset), static_cast<f32>(yoffset)});
}

auto Window::glfw_framebuffer_resize_callback(GLFWwindow* window, const i32 width, const i32 height)
    -> void {
    if (width <= 0 || height <= 0) {
        return;
    }

    auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self->m_resize_callback) {
        self->m_resize_callback(Extent2u{static_cast<u32>(width), static_cast<u32>(height)});
    }
}
} // namespace siren
