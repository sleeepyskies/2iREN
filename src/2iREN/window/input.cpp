#include "input.hpp"

#include "2iREN/utility/log.hpp"
#include "2iREN/window/window.hpp"

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

auto MouseMovement::position() const noexcept -> Point2f {
    return m_position;
}

auto MouseMovement::mouse_delta() const noexcept -> Vec2f {
    return m_mouse_delta;
}

auto MouseMovement::scroll_delta() const noexcept -> Vec2f {
    return m_scroll_delta;
}

auto MouseMovement::update() noexcept -> void {
    m_mouse_delta  = Vec2f::ZERO();
    m_scroll_delta = Vec2f::ZERO();
}

auto MouseMovement::initialize(const Point2f position) noexcept -> void {
    m_position = position;
}

auto MouseMovement::move(const Point2f position) noexcept -> void {
    m_mouse_delta += Vec2f{
        position.x - m_position.x,
        position.y - m_position.y,
    };
    m_position = position;
}

auto MouseMovement::scroll(const Vec2f delta) noexcept -> void {
    m_scroll_delta += delta;
}

auto KeyboardState::pressed(const Key key) const noexcept -> bool {
    return m_buttons.pressed(key);
}

auto KeyboardState::released(const Key key) const noexcept -> bool {
    return m_buttons.released(key);
}

auto KeyboardState::just_pressed(const Key key) const noexcept -> bool {
    return m_buttons.just_pressed(key);
}

auto KeyboardState::just_released(const Key key) const noexcept -> bool {
    return m_buttons.just_released(key);
}

auto MouseState::pressed(const Mouse button) const noexcept -> bool {
    return m_buttons.pressed(button);
}

auto MouseState::released(const Mouse button) const noexcept -> bool {
    return m_buttons.released(button);
}

auto MouseState::just_pressed(const Mouse button) const noexcept -> bool {
    return m_buttons.just_pressed(button);
}

auto MouseState::just_released(const Mouse button) const noexcept -> bool {
    return m_buttons.just_released(button);
}

auto MouseState::position() const noexcept -> Point2f {
    return m_movement.position();
}

auto MouseState::mouse_delta() const noexcept -> Vec2f {
    return m_movement.mouse_delta();
}

auto MouseState::scroll_delta() const noexcept -> Vec2f {
    return m_movement.scroll_delta();
}

Input::Input(Window& window) noexcept : m_window(window) { }

auto Input::mouse() const noexcept -> const MouseState& {
    return m_mouse;
}

auto Input::keyboard() const noexcept -> const KeyboardState& {
    return m_keyboard;
}

auto Input::cursor_mode() const noexcept -> CursorMode {
    return m_window.cursor_mode();
}

auto Input::set_cursor_mode(const CursorMode mode) -> void {
    m_window.set_cursor_mode(mode);
}

auto Input::update() noexcept -> void {
    m_keyboard.m_buttons.update();
    m_mouse.m_buttons.update();
    m_mouse.m_movement.update();
}

auto Input::press(const Key key) noexcept -> void {
    m_keyboard.m_buttons.press(key);
}

auto Input::release(const Key key) noexcept -> void {
    m_keyboard.m_buttons.release(key);
}

auto Input::press(const Mouse button) noexcept -> void {
    m_mouse.m_buttons.press(button);
}

auto Input::release(const Mouse button) noexcept -> void {
    m_mouse.m_buttons.release(button);
}

auto Input::initialize_mouse_position(const Point2f position) noexcept -> void {
    m_mouse.m_movement.initialize(position);
}

auto Input::move_mouse(const Point2f position) noexcept -> void {
    m_mouse.m_movement.move(position);
}

auto Input::scroll_mouse(const Vec2f delta) noexcept -> void {
    m_mouse.m_movement.scroll(delta);
}

} // namespace siren
