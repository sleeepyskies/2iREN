#pragma once

#include <bitset>
#include <glm/glm.hpp>

#include "2iREN/base.hpp"
#include "2iREN/input/codes.hpp"
#include "2iREN/utility/concepts.hpp"

namespace siren {

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

    BitSet m_pressed;
    BitSet m_just_pressed;
    BitSet m_just_released;
};

class MouseMovement {
public:
    [[nodiscard]] auto position() const noexcept -> glm::vec2;
    [[nodiscard]] auto mouse_delta() const noexcept -> glm::vec2;
    [[nodiscard]] auto scroll_delta() const noexcept -> glm::vec2;

private:
    friend class Input;

    auto update() noexcept -> void;

    glm::vec2 m_current_mouse_position{};
    glm::vec2 m_previous_mouse_position{};
    glm::vec2 m_mouse_delta{};

    glm::vec2 m_scroll_delta{};
    glm::vec2 m_accumulated_scroll{};
};

using MouseInput = ButtonState<Mouse>;
using KeyInput   = ButtonState<Key>;

class Input {
public:
    explicit Input(Window& window);

    auto update() noexcept -> void;

    [[nodiscard]] auto mouse() noexcept -> MouseInput&;
    [[nodiscard]] auto mouse() const noexcept -> const MouseInput&;
    [[nodiscard]] auto keyboard() noexcept -> KeyInput&;
    [[nodiscard]] auto keyboard() const noexcept -> const KeyInput&;
    [[nodiscard]] auto movement() noexcept -> MouseMovement&;
    [[nodiscard]] auto movement() const noexcept -> const MouseMovement&;
    [[nodiscard]] auto cursor_mode() const noexcept -> CursorMode;
    auto set_cursor_mode(CursorMode cursor_mode) const noexcept -> void;

private:
    Window& m_window;

    MouseInput m_mouse;
    KeyInput m_keyboard;
    MouseMovement m_movement;
};

} // namespace siren
