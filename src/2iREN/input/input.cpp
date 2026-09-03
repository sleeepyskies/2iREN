#include "input.hpp"

#include "2iREN/utility/log.hpp"
#include "2iREN/window.hpp"
#include "mappings.hpp"

namespace siren {

template <IsSizedEnum Button>
auto ButtonState<Button>::pressed(Button button) const noexcept -> bool {
    return m_pressed.test(to_index(button));
}

template <IsSizedEnum Button>
auto ButtonState<Button>::released(Button button) const noexcept -> bool {
    return !pressed(button);
}

template <IsSizedEnum Button>
auto ButtonState<Button>::just_pressed(Button button) const noexcept -> bool {
    return m_just_pressed.test(to_index(button));
}

template <IsSizedEnum Button>
auto ButtonState<Button>::just_released(Button button) const noexcept -> bool {
    return m_just_released.test(to_index(button));
}

template <IsSizedEnum Button>
auto ButtonState<Button>::update() noexcept -> void {
    m_just_pressed.reset();
    m_just_released.reset();
}

template <IsSizedEnum Button>
auto ButtonState<Button>::press(Button button) noexcept -> void {
    const auto idx = to_index(button);
    if (!m_pressed.test(idx)) {
        log::trace("Button just pressed");
        m_pressed.set(idx);
        m_just_pressed.set(idx);
    }
}

template <IsSizedEnum Button>
auto ButtonState<Button>::release(Button button) noexcept -> void {
    const auto idx = to_index(button);

    if (m_pressed.test(idx)) {
        log::trace("Button just released");
        m_pressed.reset(idx);
        m_just_released.set(idx);
    }
}

template class ButtonState<Key>;
template class ButtonState<Mouse>;

auto MouseMovement::position() const noexcept -> Vec2f { return m_current_mouse_position; }
auto MouseMovement::mouse_delta() const noexcept -> Vec2f { return m_mouse_delta; }
auto MouseMovement::scroll_delta() const noexcept -> Vec2f { return m_scroll_delta; }

auto MouseMovement::update() noexcept -> void {
    m_mouse_delta = m_current_mouse_position - m_previous_mouse_position;

    m_previous_mouse_position = m_current_mouse_position;

    m_scroll_delta       = m_accumulated_scroll;
    m_accumulated_scroll = Vec2f{0.0f};
}

Input::Input(Window& window) : m_window(window) {
    m_window.set_key_callback([this](const i32 key, const i32 action) {
        if (action == GLFW_PRESS) {
            m_keyboard.press(from_glfw_key(static_cast<u32>(key)));
        } else if (action == GLFW_RELEASE) {
            m_keyboard.release(from_glfw_key(static_cast<u32>(key)));
        }
    });

    m_window.set_mouse_button_callback([this](const i32 button, const i32 action) {
        if (action == GLFW_PRESS) {
            m_mouse.press(from_glfw_mouse(static_cast<u32>(button)));
        } else if (action == GLFW_RELEASE) {
            m_mouse.release(from_glfw_mouse(static_cast<u32>(button)));
        }
    });

    m_window.set_mouse_move_callback([this](const Vec2f position) {
        m_movement.m_current_mouse_position = position;
    });

    m_window.set_scroll_callback([this](const Vec2f delta) {
        m_movement.m_accumulated_scroll += delta;
    });
}

auto Input::update() noexcept -> void {
    m_keyboard.update();
    m_mouse.update();
    m_movement.update();
}
auto Input::mouse() noexcept -> MouseInput& { return m_mouse; }
auto Input::keyboard() noexcept -> KeyInput& { return m_keyboard; }
auto Input::movement() noexcept -> MouseMovement& { return m_movement; }
auto Input::mouse() const noexcept -> const MouseInput& { return m_mouse; }
auto Input::keyboard() const noexcept -> const KeyInput& { return m_keyboard; }
auto Input::movement() const noexcept -> const MouseMovement& { return m_movement; }
auto Input::cursor_mode() const noexcept -> CursorMode { return m_window.cursor_mode(); }
auto Input::set_cursor_mode(const CursorMode cursor_mode) const noexcept -> void {
    m_window.set_cursor_mode(cursor_mode);
}

} // namespace siren
