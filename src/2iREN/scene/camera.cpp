#include "camera.hpp"

#include <cmath>

#include "2iREN/base.hpp"
#include "2iREN/input/input.hpp"
#include "2iREN/math/angle.hpp"
#include "2iREN/math/bounded.hpp"
#include "2iREN/math/point.hpp"
#include "2iREN/math/vec3.hpp"

namespace siren {

Camera::Camera(const CameraDescriptor& descriptor) :
    m_position(descriptor.position), m_front({}), m_up({}), m_right({}), m_yaw(descriptor.yaw),
    m_pitch(descriptor.pitch), m_nearplane(descriptor.nearplane), m_farplane(descriptor.farplane),
    m_fov(descriptor.fov), m_aspect(descriptor.aspect) {
    update_vectors();
}

auto Camera::position() const noexcept -> Point3f { return m_position; }

auto Camera::front() const noexcept -> Vec3f { return m_front; }
auto Camera::right() const noexcept -> Vec3f { return m_right; }
auto Camera::up() const noexcept -> Vec3f { return m_up; }

auto Camera::yaw() const noexcept -> Degrees { return m_yaw; }
auto Camera::pitch() const noexcept -> Pitch { return m_pitch; }

auto Camera::fov() const noexcept -> Fov { return m_fov; }
auto Camera::aspect() const noexcept -> NonZeroPositiveF32 { return m_aspect; }

auto Camera::projection_view() const noexcept -> Mat4x4f {
    const auto perspective =
        Mat4x4f::perspective(m_fov.get().to_radians(), m_aspect, m_nearplane, m_farplane);

    // TODO: hack to make a point into a vector cause im lazy
    const auto positionvector = m_position - Point3f{};
    const auto tx             = -Vec3f::dot(m_right, positionvector);
    const auto ty             = -Vec3f::dot(m_up, positionvector);
    const auto tz             = Vec3f::dot(m_front, positionvector);

    // clang-format off
    const auto view = Mat4x4f{{
        m_right.x,   m_up.x,     -m_front.x,   0.0f,
        m_right.y,   m_up.y,     -m_front.y,   0.0f,
        m_right.z,   m_up.z,     -m_front.z,   0.0f,
        tx,          ty,          tz,          1.0f
    }};
    // clang-format on

    return perspective * view;
}

auto Camera::lookat(const Point3f at) noexcept -> void {
    const auto dir = Vec3f::normalize(at - m_position);
    m_yaw          = Radians{std::atan2(dir.x, dir.z)}.to_degrees();
    m_pitch        = Radians{std::asin(std::clamp(dir.y, -1.0f, 1.0f))}.to_degrees();
    update_vectors();
}

auto Camera::set_position(const Point3f position) noexcept -> void { m_position = position; }

auto Camera::set_yaw(const Degrees yaw) noexcept -> void {
    m_yaw = yaw;
    update_vectors();
}

auto Camera::set_pitch(const Pitch pitch) noexcept -> void {
    m_pitch = pitch;
    update_vectors();
}

auto Camera::set_aspect(const NonZeroPositiveF32 aspect) noexcept -> void { m_aspect = aspect; }

auto Camera::update_vectors() noexcept -> void {
    const auto yaw   = m_yaw.to_radians();
    const auto pitch = m_pitch.get().to_radians();

    m_front = Vec3f::normalize({
        std::cos(yaw.value) * std::cos(pitch.value),
        std::sin(pitch.value),
        std::sin(yaw.value) * std::cos(pitch.value),
    });

    m_right = Vec3f::normalize(Vec3f::cross(m_front, Vec3f::UP()));

    m_up = Vec3f::normalize(Vec3f::cross(m_right, m_front));
}

CameraController::CameraController(const PositiveF32 speed, const PositiveF32 sensitivity) :
    m_speed(speed), m_sensitivity(sensitivity) {}

auto CameraController::process_movement(Camera& camera, KeyInput& keys, const f32 delta) const
    -> void {

    auto movement = Vec3f{};

    if (keys.pressed(Key::W)) {
        movement += camera.front();
    }
    if (keys.pressed(Key::A)) {
        movement -= camera.right();
    }
    if (keys.pressed(Key::S)) {
        movement -= camera.front();
    }
    if (keys.pressed(Key::D)) {
        movement += camera.right();
    }
    if (keys.pressed(Key::Space)) {
        movement += Vec3f::UP();
    }
    if (keys.pressed(Key::LShift)) {
        movement -= Vec3f::UP();
    }

    // no movement to process :D
    if (movement.length() == 0) {
        return;
    }

    movement = Vec3f::normalize(movement);

    movement *= delta * m_speed.get();

    camera.set_position(Point3f::translate(camera.position(), movement));
}

auto CameraController::process_look(Camera& camera, siren::Input& input) const -> void {
    auto& mousemovement = input.movement();
    auto& mousebuttons  = input.mouse();

    if (mousebuttons.just_pressed(Mouse::Left)) {
        input.set_cursor_mode(CursorMode::Locked);
    }

    if (mousebuttons.just_released(Mouse::Left)) {
        input.set_cursor_mode(CursorMode::Visible);
    }

    if (mousebuttons.pressed(Mouse::Left)) {
        const auto offset = mousemovement.mouse_delta();
        camera.set_yaw(Degrees{camera.yaw().value + offset.x * m_sensitivity.get()});
        camera.set_pitch(Degrees{camera.pitch().get().value - offset.y * m_sensitivity.get()});
    }
}

} // namespace siren
