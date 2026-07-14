#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "2iren/base.hpp"

namespace siren {

struct PerspectiveCameraDescriptor {
    glm::vec3 position{0.f, -5.f, -5.f};
    f32 yaw{0.f};
    f32 pitch{0.f};
    f32 aspect{1280.f / 720.f};
    f32 near{0.1f};
    f32 far{1000.f};
    f32 fov{75.f};
};

class PerspectiveCamera {
public:
    explicit PerspectiveCamera(const PerspectiveCameraDescriptor& descriptor = {}) : m_descriptor(descriptor) {}

    [[nodiscard]] auto position() const noexcept -> glm::vec3;
    [[nodiscard]] auto yaw() const noexcept -> f32;
    [[nodiscard]] auto pitch() const noexcept -> f32;
    [[nodiscard]] auto aspect() const noexcept -> f32;
    [[nodiscard]] auto near() const noexcept -> f32;
    [[nodiscard]] auto far() const noexcept -> f32;
    [[nodiscard]] auto fov() const noexcept -> f32;

    auto set_position(const glm::vec3& position) noexcept -> void;
    auto set_yaw(f32 yaw) noexcept -> void;
    auto set_pitch(f32 pitch) noexcept -> void;
    auto set_aspect(f32 aspect) noexcept -> void;
    auto set_near(f32 near) noexcept -> void;
    auto set_far(f32 far) noexcept -> void;
    auto set_fov(f32 fov) noexcept -> void;

    /** @brief Returns the view matrix. This transforms world space to camera space. */
    [[nodiscard]] auto view() const noexcept -> glm::mat4;
    /** @brief Returns the projection matrix. This transforms view space to NDC. */
    [[nodiscard]] auto projection() const noexcept -> glm::mat4;
    /** @brief Returns the projection view matrix. This transforms world space to NDC. */
    [[nodiscard]] auto projection_view() const noexcept -> glm::mat4 { return projection() * view(); };

private:
    PerspectiveCameraDescriptor m_descriptor;
};

class PerspectiveCameraController {
public:
    explicit PerspectiveCameraController(const f32 speed = 5, const f32 sensitivity = 10) :
        m_sensitivity(sensitivity), m_speed(speed) {}

    auto update(PerspectiveCamera& camera) -> void;

    [[nodiscard]] auto speed() const noexcept -> f32;
    [[nodiscard]] auto sensitivity() const noexcept -> f32;

    auto set_speed(f32 speed) noexcept -> void;
    auto set_sensitivity(f32 sensitivity) noexcept -> void;

private:
    f32 m_sensitivity;
    f32 m_speed;
};

} // namespace siren
