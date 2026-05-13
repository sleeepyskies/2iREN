#pragma once

#include <functional>
#include <vector>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

#include "2iren/base.hpp"
#include "2iren/sync/mutex.hpp"
#include "fwd.hpp"


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
    std::string title = "Siren";
    /** @brief The initial width of the window */
    u32 width = 1280;
    /** @brief The initial height of the window */
    u32 height = 720;
    /** @brief If the window loads in fullscreen mode. */
    bool fullscreen = false;
    /** @brief If vsync is enabled. */
    bool vsync = true;
    /** @brief If the window has a title bar. */
    bool decorated = true;
    /** @brief If the window is resizable. */
    bool resizable = true;
    /** @brief If the window is transparent. */
    bool transparent = false;
    /** @brief Separate flag for OpenGL as it requires extra glfw hints on init. */
    bool is_opengl = true;
};

/**
 * @class Window
 * @brief A thread safe representation of an OS native window.
 * @note Atm, this acts as a semi WindowManager and single Window instance.
 * Since siren does not make use of multiple windows at the moment, this
 * is however fine.
 */
class Window {
public:
    explicit Window(const WindowDescriptor& descriptor);
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

    /**
     * @brief Sets the title of the window.
     * @param title The new window title.
     */
    auto set_title(const std::string& title) const -> void;
    /** @brief Minimizes the window. */
    auto minimize() const -> void;
    /** @brief Maximizes the window. */
    auto maximize() const -> void;
    /**
     * @brief Sets the fullscreen status of the window.
     */
    auto set_fullscreen(bool val) const -> void;
    /**
     * @brief Sets the size of the window.
     * @param size The new size of the window.
     */
    auto set_size(glm::uvec2 size) const -> void;
    /**
     * @brief Sets the position of the window.
     * @param position The new position of the window.
     */
    auto set_position(glm::ivec2 position) const -> void;
    /**
     * @brief Processes the internal request queue and polls for any OS events
     * @warning This must only be called from the main thread!!!
     */
    auto poll_events() const -> void;

private:
    /** @brief Callback function type used internally to defer execution of certain requests. */
    using WindowRequest = std::function<void()>;

    GLFWwindow* m_handle;
    mutable std::atomic<WindowMode> m_window_mode;
    Mutex<glm::uvec2> m_size;
    Mutex<glm::ivec2> m_position;
    Mutex<std::string> m_title;
    Mutex<std::vector<WindowRequest>> m_requests;
    // SwapChain m_swapchain;
};

} // namespace siren
