#pragma once

#include <functional>

#include "2iREN/core/base.hpp"
#include "2iREN/math/bounded.hpp"
#include "2iREN/math/extent.hpp"
#include "2iREN/math/point2.hpp"
#include "2iREN/window/codes.hpp"
#include "2iREN/window/input.hpp"

struct GLFWwindow;

namespace siren {

/// @brief Represents the various states a window can be in.
enum class WindowMode {
    /// @brief Standard window mode.
    Normal,
    /// @brief The window is minimized, meaning not visible and on the taskbar.
    Minimized,
    /// @brief The window is maximized. Not to be confused with fullscreen.
    Maximized,
    /// @brief The window is fullscreen. GPU has fully taken over the monitor.
    Fullscreen
};

/// @brief Window parameters used to initialize the @ref Window.
struct WindowDescriptor {
    /// @brief The initial title of the window.
    std::string title = "2iREN";
    /// @brief The initial width of the window
    u32 width = 1280;
    /// @brief The initial height of the window
    u32 height = 720;
    /// @brief If the window has a title bar.
    bool decorated = true;
    /// @brief If the window is resizable.
    bool resizable = true;
    /// @brief If the window is transparent.
    bool transparent = false;
    /// @brief The initial mode the window should load as.
    WindowMode mode = WindowMode::Normal;
};

using OnResizeCallback = std::function<void(Extent2u)>;

class Window {
public:
    ~Window();

    Window(const Window&)                      = delete;
    Window(Window&& other)                     = delete;
    Window& operator=(const Window&)           = delete;
    Window& operator=(Window&& other) noexcept = delete;

    /// @brief Returns the aspect ratio of this window.
    [[nodiscard]]
    auto aspect() const noexcept -> NonZeroPositiveF32;

    /// @brief Returns the current extent of the window.
    [[nodiscard]]
    auto extent() const noexcept -> Extent2u;

    /// @brief Returns the current extent of the windows framebuffer.
    [[nodiscard]]
    auto framebuffer_extent() const noexcept -> Extent2u;

    /// @brief Returns the current position of the window.
    [[nodiscard]]
    auto position() const noexcept -> Point2u;

    /// @brief Returns the current title of the window.
    [[nodiscard]]
    auto title() const noexcept -> std::string;

    /// @brief Returns the current window mode.
    [[nodiscard]]
    auto mode() const noexcept -> WindowMode;

    /// @brief Returns the current @ref CursorMode of the window.
    [[nodiscard]]
    auto cursor_mode() const noexcept -> CursorMode;

    /// @brief Checks whether the window should close.
    [[nodiscard]]
    auto should_close() const noexcept -> bool;

    /// @brief Returns the native window handle.
    [[nodiscard]]
    auto native_handle() const noexcept -> GLFWwindow*;

    /// @brief Returns the current input state.
    [[nodiscard]]
    auto input() const noexcept -> const Input&;

    /// @brief Sets the title of the window.
    auto set_title(const std::string& title) -> void;

    /// @brief Sets the window mode.
    auto set_mode(const WindowMode mode) -> void;

    /// @brief Sets the extent of the window.
    auto set_extent(Extent2u extent) -> void;

    /// @brief Sets the position of the window.
    auto set_position(Point2u position) -> void;

    /// @brief Sets the @ref CursorMode of the window.
    auto set_cursor_mode(CursorMode mode) -> void;

    /// @brief Polls OS events and updates the current input state.
    auto poll_events() -> void;

    /// @brief Sets the callback invoked when the framebuffer is resized.
    auto on_resize(OnResizeCallback&& callback) -> void;

private:
    friend class Context;
    Window(const WindowDescriptor& descriptor);

    static auto glfw_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
        -> void;
    static auto glfw_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods)
        -> void;
    static auto glfw_mouse_move_callback(GLFWwindow* window, f64 xpos, f64 ypos) -> void;
    static auto glfw_scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) -> void;
    static auto glfw_framebuffer_resize_callback(GLFWwindow* window, i32 width, i32 height) -> void;

    GLFWwindow* m_handle = nullptr;
    Input m_input;
    OnResizeCallback m_resize_callback;
};

} // namespace siren
