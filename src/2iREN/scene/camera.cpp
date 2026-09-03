#include "camera.hpp"

#include <cmath>

#include "2iREN/base.hpp"
#include "2iREN/input/input.hpp"
#include "2iREN/math/angle.hpp"
#include "2iREN/math/bounded.hpp"
#include "2iREN/math/point.hpp"

namespace siren {

Camera::Camera(const CameraDescriptor& descriptor) :
    m_position(descriptor.position), m_front({}), m_up({}), m_right({}), m_yaw(descriptor.yaw),
    m_pitch(descriptor.pitch), m_nearplane(descriptor.nearplane), m_farplane(descriptor.farplane),
    m_fov(descriptor.fov) {
    update_vectors();
}

auto Camera::position() const noexcept -> Point3f { return m_position; }

auto Camera::front() const noexcept -> Vec3f { return m_front; }

auto Camera::right() const noexcept -> Vec3f { return m_right; }

auto Camera::up() const noexcept -> Vec3f { return m_up; }

auto Camera::yaw() const noexcept -> Yaw { return m_yaw; }

auto Camera::pitch() const noexcept -> Degrees { return m_pitch; }

auto Camera::fov() const noexcept -> Fov { return m_fov; }

auto Camera::projection_view(const NonZeroPositiveF32 aspect_ratio) const noexcept -> Mat4x4f {
    const auto perspective =
        Mat4x4f::perspective(m_fov.get().to_radians(), aspect_ratio, m_nearplane, m_farplane);

    // TODO: hack cause im lazy AKJHFSJK
    const f32 tx = -Vec3f::dot(m_right, m_position - Point3f{});
    const f32 ty = -Vec3f::dot(m_up, m_position - Point3f{});
    const f32 tz = Vec3f::dot(m_front, m_position - Point3f{});

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

auto Camera::set_position(const Point3f position) noexcept -> void { m_position = position; }

auto Camera::set_yaw(const Yaw yaw) noexcept -> void {
    m_yaw = yaw;
    update_vectors();
}

auto Camera::set_pitch(const Degrees pitch) noexcept -> void {
    m_pitch = pitch;
    update_vectors();
}

auto Camera::update_vectors() noexcept -> void {
    const auto yaw   = m_yaw.get().to_radians();
    const auto pitch = m_pitch.to_radians();

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
        movement += Vec3f{0, 0, -1};
    }
    if (keys.pressed(Key::A)) {
        movement += Vec3f{-1, 0, 0};
    }
    if (keys.pressed(Key::S)) {
        movement += Vec3f{0, 0, 1};
    }
    if (keys.pressed(Key::D)) {
        movement += Vec3f{1, 0, 0};
    }
    if (keys.pressed(Key::Space)) {
        movement += Vec3f{0, 1, 0};
    }
    if (keys.pressed(Key::LShift)) {
        movement += Vec3f{0, -1, 0};
    }

    movement *= delta;

    camera.set_position(Point3f::translate(camera.position(), movement));
}

auto CameraController::process_look(Camera& camera, MouseMovement& mouse, const f32 delta) const
    -> void {
    const auto offset = mouse.mouse_delta();

    camera.set_yaw(Degrees{(camera.yaw().get().value + offset.x) * delta});
    camera.set_pitch(Degrees{(camera.pitch().value + offset.y) * delta});
}

} // namespace siren
