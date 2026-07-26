#include "camera.hpp"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "2iren/input/input.hpp"
#include "log.hpp"
#include "time.hpp"

namespace siren {

auto PerspectiveCamera::position() const noexcept -> glm::vec3 { return m_descriptor.position; }
auto PerspectiveCamera::yaw() const noexcept -> f32 { return m_descriptor.yaw; }
auto PerspectiveCamera::pitch() const noexcept -> f32 { return m_descriptor.pitch; }
auto PerspectiveCamera::aspect() const noexcept -> f32 { return m_descriptor.aspect; }
auto PerspectiveCamera::near() const noexcept -> f32 { return m_descriptor.near; }
auto PerspectiveCamera::far() const noexcept -> f32 { return m_descriptor.far; }
auto PerspectiveCamera::fov() const noexcept -> f32 { return m_descriptor.fov; }

auto PerspectiveCamera::set_position(const glm::vec3& position) noexcept -> void { m_descriptor.position = position; }
auto PerspectiveCamera::set_yaw(const f32 yaw) noexcept -> void { m_descriptor.yaw = yaw; }
auto PerspectiveCamera::set_pitch(const f32 pitch) noexcept -> void { m_descriptor.pitch = pitch; }
auto PerspectiveCamera::set_aspect(const f32 aspect) noexcept -> void { m_descriptor.aspect = aspect; }
auto PerspectiveCamera::set_near(const f32 near) noexcept -> void { m_descriptor.near = near; }
auto PerspectiveCamera::set_far(const f32 far) noexcept -> void { m_descriptor.far = far; }
auto PerspectiveCamera::set_fov(const f32 fov) noexcept -> void { m_descriptor.fov = fov; }

auto PerspectiveCamera::look_at(const glm::vec3 point) -> void {
    const auto dir = glm::normalize(point - m_descriptor.position);
    m_descriptor.yaw = std::atan2(dir.x, dir.z);
    m_descriptor.pitch = std::asin(dir.y);
}

auto PerspectiveCamera::view() const noexcept -> glm::mat4 {
    const glm::vec3 direction{cos(glm::radians(m_descriptor.yaw)) * cos(glm::radians(m_descriptor.pitch)),
        sin(glm::radians(m_descriptor.pitch)),
        sin(glm::radians(m_descriptor.yaw)) * cos(glm::radians(m_descriptor.pitch))};
    return glm::lookAt(m_descriptor.position, m_descriptor.position + direction, glm::vec3(0, 1, 0));
}

auto PerspectiveCamera::projection() const noexcept -> glm::mat4 {
    return glm::perspective(glm::radians(m_descriptor.fov), m_descriptor.aspect, m_descriptor.near, m_descriptor.far);
}

auto PerspectiveCameraController::update(PerspectiveCamera& camera, const Input& input) -> void {
    const f32 dt  = time::delta_s();
    auto position = camera.position();

    const f32 yaw   = glm::radians(camera.yaw());
    const f32 pitch = glm::radians(camera.pitch());
    const glm::vec3 forward{cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch)};
    const glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));

    glm::vec3 movement{0.0f};
    if (input.keyboard().pressed(Key::Space)) {
        movement += glm::vec3{0, 1, 0};
    }
    if (input.keyboard().pressed(Key::LShift)) {
        movement -= glm::vec3{0, 1, 0};
    }
    if (input.keyboard().pressed(Key::W)) {
        movement += forward;
    }
    if (input.keyboard().pressed(Key::S)) {
        movement -= forward;
    }
    if (input.keyboard().pressed(Key::D)) {
        movement += right;
    }
    if (input.keyboard().pressed(Key::A)) {
        movement -= right;
    }
    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement);
    }

    position += movement * static_cast<float>(m_speed * dt);
    camera.set_position(position);

    if (input.mouse().just_pressed(Mouse::Left)) {
        input.set_cursor_mode(CursorMode::Locked);
    }
    if (input.mouse().just_released(Mouse::Left)) {
        input.set_cursor_mode(CursorMode::Visible);
    }

    // rotate continuously while held, not just on the click frame
    if (input.mouse().pressed(Mouse::Left)) {
        const glm::vec2 mouse_delta = input.movement().mouse_delta();
        camera.set_yaw(camera.yaw() + mouse_delta.x * m_sensitivity);
        camera.set_pitch(glm::clamp(float(camera.pitch() - mouse_delta.y * m_sensitivity), -89.0f, 89.0f));
    }
}

auto PerspectiveCameraController::speed() const noexcept -> f32 { return m_speed; }
auto PerspectiveCameraController::sensitivity() const noexcept -> f32 { return m_sensitivity; }

auto PerspectiveCameraController::set_speed(const f32 speed) noexcept -> void { m_speed = speed; }
auto PerspectiveCameraController::set_sensitivity(const f32 sensitivity) noexcept -> void {
    m_sensitivity = sensitivity;
}

} // namespace siren
