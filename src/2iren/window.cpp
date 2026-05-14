#include "window.hpp"

#include <GLFW/glfw3.h>
#include <libassert/assert.hpp>

#include "context.hpp"
#include "util/log.hpp"

/// @todo:
///     - Window resize has static data in function -> only works with one instance
///     - Window initializes and shutsdown GLFW, is fine for now and forseeable future
///       but maybe would be better to have a context or something that manages lifetime

namespace siren {

Window::Window(const WindowDescriptor& descriptor)  {
    GLFWmonitor* monitor = nullptr;
    if (descriptor.fullscreen) {
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

    // don't set vsync here, render thread should do this since its context dependent

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

auto Window::handle() const noexcept -> GLFWwindow* {
    return m_handle;
}

auto Window::width() const noexcept -> u32 {
    return size().x;
}

auto Window::height() const noexcept -> u32 {
    return size().y;
}

auto Window::size() const noexcept -> glm::uvec2 {
    return m_size.get();
}

auto Window::position() const noexcept -> glm::ivec2 {
    return m_position.get();
}

auto Window::title() const noexcept -> std::string {
    return m_title.get();
}

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

auto Window::set_title(const std::string& title) const -> void {
    m_requests.lock()->emplace_back([this, title]{ glfwSetWindowTitle(m_handle, title.c_str()); });
}

auto Window::minimize() const -> void {
    m_requests.lock()->emplace_back([this]{ glfwIconifyWindow(m_handle); });
}

auto Window::maximize() const -> void {
    m_requests.lock()->emplace_back([this]{ glfwMaximizeWindow(m_handle); });
}

auto Window::set_fullscreen(const bool val) const -> void {
    // setting to same, so skip
    if ((val && m_window_mode == WindowMode::Fullscreen) || (!val && m_window_mode != WindowMode::Fullscreen)) {
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

    m_requests.lock()->emplace_back(
        [this, val]{
            if (val) {
                const auto monitor = glfwGetPrimaryMonitor();
                const auto mode    = glfwGetVideoMode(monitor);
                glfwSetWindowMonitor(m_handle, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                this->m_window_mode = WindowMode::Fullscreen;
            } else {
                glfwSetWindowMonitor(m_handle, nullptr, cached_x, cached_y, (i32)cached_w, (i32)cached_h, 0);
            }
        }
    );
}

auto Window::set_size(glm::uvec2 size) const -> void {
    m_requests.lock()->emplace_back(
        [this, size]{ glfwSetWindowSize(m_handle, static_cast<i32>(size.x), static_cast<i32>(size.y)); }
    );
}

auto Window::set_position(glm::ivec2 position) const -> void {
    m_requests.lock()->emplace_back([this, position]{ glfwSetWindowPos(m_handle, position.x, position.y); });
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

} // namespace siren
