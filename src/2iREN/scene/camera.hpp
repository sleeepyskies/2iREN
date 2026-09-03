#pragma once

#include "2iREN/base.hpp"
#include "2iREN/input/input.hpp"
#include "2iREN/math/angle.hpp"
#include "2iREN/math/bounded.hpp"
#include "2iREN/math/mat4x4.hpp"
#include "2iREN/math/point.hpp"
#include "2iREN/math/vec3.hpp"

namespace siren {

/// @brief A degree value clamped to the range [-89, 89] represting camera yaw.
using Yaw = Bounded<Degrees, Degrees{-89.f}, Degrees{89.f}, ClampBoundsPolicy>;
/// @brief A degree value clamped to [0, 500] representing the camera fov.
using Fov = Bounded<Degrees, Degrees{0}, Degrees{500}, ClampBoundsPolicy>;

struct CameraDescriptor {
    Point3f position = {0.f, 0.f, 0.f};
    Degrees pitch    = Degrees{0.f};
    Yaw yaw          = Degrees{0.f};
    Fov fov          = Degrees{75.f};
    f32 nearplane    = 0.1f;
    f32 farplane     = 100.f;
};

class Camera {
public:
    explicit Camera(const CameraDescriptor& descriptor);

    auto position() const noexcept -> Point3f;
    auto front() const noexcept -> Vec3f;
    auto up() const noexcept -> Vec3f;
    auto right() const noexcept -> Vec3f;
    auto yaw() const noexcept -> Yaw;
    auto pitch() const noexcept -> Degrees;
    auto fov() const noexcept -> Fov;
    auto projection_view(const NonZeroPositiveF32 aspect_ratio) const noexcept -> Mat4x4f;

    auto set_position(Point3f position) noexcept -> void;
    auto set_yaw(Yaw yaw) noexcept -> void;
    auto set_pitch(Degrees pitch) noexcept -> void;

private:
    auto update_vectors() noexcept -> void;

private:
    Point3f m_position;
    Vec3f m_front;
    Vec3f m_up;
    Vec3f m_right;

    Yaw m_yaw;
    Degrees m_pitch;

    f32 m_nearplane;
    f32 m_farplane;
    Fov m_fov;
};

class CameraController {
public:
    CameraController(const PositiveF32 speed, const PositiveF32 m_sensitivity);

    auto process_movement(Camera& camera, KeyInput& keys, const f32 delta) const -> void;
    auto process_look(Camera& camera, MouseMovement& mouse, const f32 delta) const -> void;

private:
    PositiveF32 m_speed;
    PositiveF32 m_sensitivity;
};

} // namespace siren
