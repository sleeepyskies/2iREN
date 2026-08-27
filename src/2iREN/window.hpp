#pragma once

#include <GLFW/glfw3.h>
#include <atomic>
#include <functional>
#include <glm/vec2.hpp>
#include <vector>

#include "2iREN/base.hpp"
#include "2iREN/sync/mutex.hpp"
#include "input/codes.hpp"

namespace siren {

/**
 * @brief Represents a state a @ref Window can be in. Iff the window exists,
 * it must be in one of these states.
 */
enum class WindowMode {
    /** @brief Standard window mode. */
    Normal,
    /** @brief The window is minimized, meaning not visible and on the taskbar. */
    Minimized,
    /** @brief The window is maximized. Not to be confused with fullscreen. */
    Maximized,
    /** @brief The window is fullscreen. GPU has fully taken over the monitor. */
    Fullscreen
};

/**
 * @brief Window parameters used to initialize the @ref Window.
 */
struct WindowDescriptor {
    /** @brief The initial title of the window. */
    std::string title = "2iREN";
    /** @brief The initial width of the window */
    u32 width = 1280;
    /** @brief The initial height of the window */
    u32 height = 720;
    /** @brief If the window has a title bar. */
    bool decorated = true;
    /** @brief If the window is resizable. */
    bool resizable = true;
    /** @brief If the window is transparent. */
    bool transparent = false;
    /** @brief The initial mode the window should load as. */
    WindowMode initial_mode = WindowMode::Normal;
};

using KeyCallback         = std::function<void(int key, int action)>;
using MouseButtonCallback = std::function<void(int button, int action)>;
using MouseMoveCallback   = std::function<void(glm::vec2)>;
using ScrollCallback      = std::function<void(glm::vec2)>;
using OnResizeCallback    = std::function<void(glm::ivec2)>;

/**
 * @class Window
 * @brief A thread safe representation of an OS native window.
 * @note Atm, this acts as a semi WindowManager and single Window instance.
 * Since siren does not make use of multiple windows at the moment, this
 * is however fine.
 * @warning Must be created on the main thread!
 * @todo Add glfw callbacks, we never update window state atm.
 */
class Window {
public:
    ~Window();

    Window(const Window&)                      = delete;
    Window(Window&& other)                     = delete;
    Window& operator=(const Window&)           = delete;
    Window& operator=(Window&& other) noexcept = delete;

    /** @brief Returns a raw handle to the underlying GLFW window */
    [[nodiscard]] auto handle() const noexcept -> GLFWwindow*;
    /** @brief Returns the current width of the window. */
    [[nodiscard]] auto width() const noexcept -> u32;
    /** @brief Returns the current height of the window. */
    [[nodiscard]] auto height() const noexcept -> u32;
    /** @brief Returns the current size of the window. */
    [[nodiscard]] auto size() const noexcept -> glm::uvec2;
    /** @brief Returns the current position of the window. */
    [[nodiscard]] auto position() const noexcept -> glm::ivec2;
    /** @brief Returns the current title of the window. */
    [[nodiscard]] auto title() const noexcept -> std::string;
    /** @brief Checks whether the window is currently minimized. */
    [[nodiscard]] auto is_minimized() const noexcept -> bool;
    /** @brief Checks whether the window is currently maximized. */
    [[nodiscard]] auto is_maximized() const noexcept -> bool;
    /** @brief Checks whether the window is currently fullscreen. */
    [[nodiscard]] auto is_fullscreen() const noexcept -> bool;
    /** @brief Checks whether the window should close. */
    [[nodiscard]] auto should_close() const noexcept -> bool;
    /** @brief Returns the current @ref CursorMode of the window. */
    [[nodiscard]] auto cursor_mode() const noexcept -> CursorMode;

    /** @brief Sets the title of the window. */
    auto set_title(const std::string& title) const -> void;
    /** @brief Minimizes the window. */
    auto minimize() const -> void;
    /** @brief Maximizes the window. */
    auto maximize() const -> void;
    /** @brief Sets the fullscreen status of the window. */
    auto set_fullscreen(bool val) const -> void;
    /** @brief Sets the size of the window. */
    auto set_size(glm::uvec2 size) const -> void;
    /** @brief Sets the position of the window. */
    auto set_position(glm::ivec2 position) const -> void;
    /** @brief Sets the @ref CursorMode of the window. */
    auto set_cursor_mode(CursorMode mode) const noexcept -> void;
    /**
     * @brief Processes the internal request queue and polls for any OS events
     * @warning This must only be called from the main thread!!!
     */
    auto poll_events() const -> void;

    auto on_resize(OnResizeCallback&& callback) -> void;

    auto set_key_callback(KeyCallback&& callback) -> void;
    auto set_mouse_button_callback(MouseButtonCallback&& callback) -> void;
    auto set_mouse_move_callback(MouseMoveCallback&& callback) -> void;
    auto set_scroll_callback(ScrollCallback&& callback) -> void;

private:
    friend class Context;
    explicit Window(const WindowDescriptor& descriptor = {});

    /** @brief Callback function type used internally to defer execution of certain requests. */
    using WindowRequest = std::function<void()>;

    GLFWwindow* m_handle;
    mutable std::atomic<WindowMode> m_window_mode;
    mutable std::atomic<CursorMode> m_cursor_mode;
    Mutex<glm::uvec2> m_size;
    Mutex<glm::ivec2> m_position;
    Mutex<std::string> m_title;
    Mutex<std::vector<WindowRequest>> m_requests;

    KeyCallback m_key_callback;
    MouseButtonCallback m_mouse_button_callback;
    MouseMoveCallback m_mouse_move_callback;
    ScrollCallback m_scroll_callback;
    OnResizeCallback m_resize_callback;

    static auto glfw_key_callback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods) -> void;
    static auto glfw_mouse_button_callback(GLFWwindow* window, i32 button, i32 action, i32 mods) -> void;
    static auto glfw_mouse_move_callback(GLFWwindow* window, f64 xpos, f64 ypos) -> void;
    static auto glfw_scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) -> void;
    static auto glfw_window_resize_callback(GLFWwindow* window, i32 w, i32 h) -> void;
};

} // namespace siren
