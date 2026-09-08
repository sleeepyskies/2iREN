#pragma once

#include <bitset>

#include "2iREN/core/base.hpp"
#include "2iREN/math/point2.hpp"
#include "2iREN/math/vec2.hpp"
#include "2iREN/utility/concepts.hpp"
#include "2iREN/window/codes.hpp"

namespace siren {

class Input;
class Window;

template <IsSizedEnum Button>
class ButtonState {
    static constexpr usize Size = std::to_underlying(Button::Max);
    using BitSet                = std::bitset<Size>;

public:
    [[nodiscard]] auto pressed(Button button) const noexcept -> bool;
    [[nodiscard]] auto released(Button button) const noexcept -> bool;
    [[nodiscard]] auto just_pressed(Button button) const noexcept -> bool;
    [[nodiscard]] auto just_released(Button button) const noexcept -> bool;

    constexpr auto to_index(Button button) const noexcept -> usize {
        return std::to_underlying(button);
    }

private:
    friend class Input;

    auto update() noexcept -> void;
    auto press(Button button) noexcept -> void;
    auto release(Button button) noexcept -> void;

    BitSet m_pressed{};
    BitSet m_just_pressed{};
    BitSet m_just_released{};
};

class MouseMovement {
public:
    [[nodiscard]] auto position() const noexcept -> Point2f;
    [[nodiscard]] auto mouse_delta() const noexcept -> Vec2f;
    [[nodiscard]] auto scroll_delta() const noexcept -> Vec2f;

private:
    friend class Input;

    auto update() noexcept -> void;
    auto initialize(Point2f position) noexcept -> void;
    auto move(Point2f position) noexcept -> void;
    auto scroll(Vec2f delta) noexcept -> void;

    Point2f m_position{};
    Vec2f m_mouse_delta{};
    Vec2f m_scroll_delta{};
};

using MouseInput = ButtonState<Mouse>;
using KeyInput   = ButtonState<Key>;

class KeyboardState {
public:
    [[nodiscard]] auto pressed(Key key) const noexcept -> bool;
    [[nodiscard]] auto released(Key key) const noexcept -> bool;
    [[nodiscard]] auto just_pressed(Key key) const noexcept -> bool;
    [[nodiscard]] auto just_released(Key key) const noexcept -> bool;

private:
    friend class Input;

    KeyInput m_buttons{};
};

class MouseState {
public:
    [[nodiscard]] auto pressed(Mouse button) const noexcept -> bool;
    [[nodiscard]] auto released(Mouse button) const noexcept -> bool;
    [[nodiscard]] auto just_pressed(Mouse button) const noexcept -> bool;
    [[nodiscard]] auto just_released(Mouse button) const noexcept -> bool;

    [[nodiscard]] auto position() const noexcept -> Point2f;
    [[nodiscard]] auto mouse_delta() const noexcept -> Vec2f;
    [[nodiscard]] auto scroll_delta() const noexcept -> Vec2f;

private:
    friend class Input;

    MouseInput m_buttons;
    MouseMovement m_movement;
};

/// @brief Grouping of all input data held by a window.
class Input {
public:
    [[nodiscard]]
    auto mouse() const noexcept -> const MouseState&;
    [[nodiscard]]
    auto keyboard() const noexcept -> const KeyboardState&;

    [[nodiscard]]
    auto cursor_mode() const noexcept -> CursorMode;
    auto set_cursor_mode(CursorMode mode) -> void;

private:
    friend class Window;

    explicit Input(Window& window) noexcept;

    auto update() noexcept -> void;
    auto press(Key key) noexcept -> void;
    auto release(Key key) noexcept -> void;
    auto press(Mouse button) noexcept -> void;
    auto release(Mouse button) noexcept -> void;
    auto initialize_mouse_position(Point2f position) noexcept -> void;
    auto move_mouse(Point2f position) noexcept -> void;
    auto scroll_mouse(Vec2f delta) noexcept -> void;

    Window& m_window;
    MouseState m_mouse;
    KeyboardState m_keyboard;
};

} // namespace siren
