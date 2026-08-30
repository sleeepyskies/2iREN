#include "window.hpp"

#include <GLFW/glfw3.h>

#include "2iREN/core/assert.hpp"
#include "context.hpp"
#include "input/mappings.hpp"
#include "2iREN/utility/log.hpp"

/// @todo:
///     - Window resize has static data in function -> only works with one instance
///     - Window initializes and shutsdown GLFW, is fine for now and forseeable future
///       but maybe would be better to have a context or something that manages lifetime

namespace siren {
Window::Window(const WindowDescriptor& descriptor) {
    GLFWmonitor* monitor = nullptr;

    {
        const auto platform = glfwGetPlatform();
        if (platform == GLFW_PLATFORM_X11) {
            log::info("Using windowing platform X11");
        } else if (platform == GLFW_PLATFORM_WAYLAND) {
            log::info("Using windowing platform Wayland");
        } else if (platform == GLFW_PLATFORM_COCOA) {
            log::info("Using windowing platform Cocoa");
        } else if (platform == GLFW_PLATFORM_WIN32) {
            log::info("Using windowing platform Win32");
        }
    }

    if (descriptor.initial_mode == WindowMode::Fullscreen) {
        monitor = glfwGetPrimaryMonitor();
    }

    if (descriptor.decorated) {
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    }

    if (descriptor.resizable) {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    if (descriptor.transparent) {
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    } else {
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    }

    m_handle = glfwCreateWindow(
        (i32)descriptor.width, (i32)descriptor.height, descriptor.title.c_str(), monitor, nullptr
    );
    ASSERT(m_handle, "Failed to create GLFW window");

    // setup callbacks
    glfwSetWindowUserPointer(m_handle, this);
    glfwSetKeyCallback(m_handle, glfw_key_callback);
    glfwSetMouseButtonCallback(m_handle, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(m_handle, GLFWcursorposfun(glfw_mouse_move_callback));
    glfwSetScrollCallback(m_handle, GLFWscrollfun(glfw_scroll_callback));
    glfwSetWindowSizeCallback(m_handle, glfw_window_resize_callback);

    // don't set vsync here, render thread should do this since its context dependent

    // take over values
    {
        m_window_mode = descriptor.initial_mode;

        i32 w, h;
        glfwGetWindowSize(m_handle, &w, &h);
        m_size.set(glm::uvec2(w, h));

        i32 x, y;
        glfwGetWindowPos(m_handle, &x, &y);
        m_position.set(glm::uvec2(x, y));

        m_title.set(descriptor.title);
    }

    log::info("Window created successfully: {}x{}", descriptor.width, descriptor.height);
    glfwDefaultWindowHints();
    glfwMakeContextCurrent(nullptr);
}

Window::~Window() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
    // todo: once many windows are supported this should go in the WindowManager or something
    glfwTerminate();
}

auto Window::handle() const noexcept -> GLFWwindow* { return m_handle; }
auto Window::width() const noexcept -> u32 { return size().x; }
auto Window::height() const noexcept -> u32 { return size().y; }
auto Window::size() const noexcept -> glm::uvec2 { return m_size.get(); }
auto Window::position() const noexcept -> glm::ivec2 { return m_position.get(); }
auto Window::title() const noexcept -> std::string { return m_title.get(); }
auto Window::is_minimized() const noexcept -> bool {
    return m_window_mode.load() == WindowMode::Minimized;
}
auto Window::is_maximized() const noexcept -> bool {
    return m_window_mode.load() == WindowMode::Maximized;
}
auto Window::is_fullscreen() const noexcept -> bool {
    return m_window_mode.load() == WindowMode::Fullscreen;
}
auto Window::should_close() const noexcept -> bool {
    return m_handle == nullptr || glfwWindowShouldClose(m_handle);
}
auto Window::cursor_mode() const noexcept -> CursorMode { return m_cursor_mode; }

auto Window::set_title(const std::string& title) const -> void {
    m_requests.lock()->emplace_back([this, title] {
        this->m_title.set(title);
        glfwSetWindowTitle(m_handle, title.c_str());
        log::trace("Window title set to {}", title);
    });
}

auto Window::minimize() const -> void {
    m_requests.lock()->emplace_back([this] {
        this->m_window_mode = WindowMode::Minimized;
        glfwIconifyWindow(m_handle);
        log::trace("Window minimized");
    });
}

auto Window::maximize() const -> void {
    m_requests.lock()->emplace_back([this] {
        this->m_window_mode = WindowMode::Maximized;
        glfwMaximizeWindow(m_handle);
        log::trace("Window maximized");
    });
}

auto Window::set_fullscreen(const bool val) const -> void {
    // setting to same, so skip
    if ((val && m_window_mode == WindowMode::Fullscreen) ||
        (!val && m_window_mode != WindowMode::Fullscreen)) {
        return;
    }

    // store vals so we remember what to go back to on fs exit
    static i32 cached_x, cached_y;
    static u32 cached_w, cached_h;
    if (val) {
        cached_x = position().x;
        cached_y = position().y;
        cached_w = size().x;
        cached_h = size().y;
    }

    m_requests.lock()->emplace_back([this, val] {
        if (val) {
            const auto monitor = glfwGetPrimaryMonitor();
            const auto mode    = glfwGetVideoMode(monitor);
            glfwSetWindowMonitor(
                m_handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate
            );
            this->m_window_mode = WindowMode::Fullscreen;
            log::trace("Window set to fullscreen");
        } else {
            glfwSetWindowMonitor(
                m_handle, nullptr, cached_x, cached_y, (i32)cached_w, (i32)cached_h, 0
            );
            this->m_window_mode = WindowMode::Normal;
            log::trace("Window set to normal");
        }
    });
}

auto Window::set_size(glm::uvec2 size) const -> void {
    m_requests.lock()->emplace_back([this, size] {
        glfwSetWindowSize(m_handle, static_cast<i32>(size.x), static_cast<i32>(size.y));
        this->m_size.set(size);
        log::trace("Window size set to ({}, {})", size.x, size.y);
    });
}

auto Window::set_position(glm::ivec2 position) const -> void {
    m_requests.lock()->emplace_back([this, position] {
        glfwSetWindowPos(m_handle, position.x, position.y);
        this->m_position.set(position);
        log::trace("Window position set to ({}, {})", position.x, position.y);
    });
}

auto Window::set_cursor_mode(CursorMode mode) const noexcept -> void {
    m_requests.lock()->emplace_back([this, mode] {
        glfwSetInputMode(m_handle, GLFW_CURSOR, to_glfw(mode));
        this->m_cursor_mode = mode;
        log::trace("Cursor mode set to {}", (i32)mode);
    });
}

auto Window::poll_events() const -> void {
    // first, we handle any requests that were made in the previous frame
    std::vector<WindowRequest> requests;
    {
        auto guard = m_requests.lock();
        requests   = std::move(*guard);
        guard->clear();
    }

    for (const auto& request : requests) {
        request();
    }

    // poll events
    glfwPollEvents();
}

auto Window::on_resize(OnResizeCallback&& callback) -> void {
    m_resize_callback = std::move(callback);
}

auto Window::set_key_callback(KeyCallback&& callback) -> void {
    m_key_callback = std::move(callback);
}

auto Window::set_mouse_button_callback(MouseButtonCallback&& callback) -> void {
    m_mouse_button_callback = std::move(callback);
}

auto Window::set_mouse_move_callback(MouseMoveCallback&& callback) -> void {
    m_mouse_move_callback = std::move(callback);
}

auto Window::set_scroll_callback(ScrollCallback&& callback) -> void {
    m_scroll_callback = std::move(callback);
}

void Window::glfw_key_callback(
    GLFWwindow* window, const i32 key, const i32, const i32 action, const i32
) {
    const auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self->m_key_callback) {
        self->m_key_callback(key, action);
    }
}

auto Window::glfw_mouse_button_callback(
    GLFWwindow* window, const i32 button, const i32 action, const i32
) -> void {
    const auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self->m_mouse_button_callback) {
        self->m_mouse_button_callback(button, action);
    }
}

auto Window::glfw_mouse_move_callback(GLFWwindow* window, const f64 xpos, const f64 ypos) -> void {
    const auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self->m_mouse_move_callback) {
        self->m_mouse_move_callback(glm::vec2{static_cast<f32>(xpos), static_cast<f32>(ypos)});
    }
}

auto Window::glfw_scroll_callback(GLFWwindow* window, const f64 xoffset, const f64 yoffset)
    -> void {
    const auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (self->m_scroll_callback) {
        self->m_scroll_callback(glm::vec2{static_cast<f32>(xoffset), static_cast<f32>(yoffset)});
    }
}

auto Window::glfw_window_resize_callback(GLFWwindow* window, const i32 w, const i32 h) -> void {
    const auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
    self->m_size.set(glm::uvec2{w, h});
    if (self->m_resize_callback) {
        self->m_resize_callback(glm::ivec2{static_cast<f32>(w), static_cast<f32>(h)});
    }
}
} // namespace siren
