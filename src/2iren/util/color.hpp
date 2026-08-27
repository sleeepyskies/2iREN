#pragma once

#include <format>
#include <glm/glm.hpp>
#include <string>

#include "2iren/base.hpp"

namespace siren {
/**
 * @struct Rgba
 * @brief A four channel color.
 */
struct Rgba {
    /** @brief The red color channel. */
    f32 r;
    /** @brief The green color channel. */
    f32 g;
    /** @brief The blue color channel. */
    f32 b;
    /** @brief The alpha channel. */
    f32 a;

    /** @brief Construct from an existing GLM vector. */
    constexpr explicit Rgba(const glm::vec4& v) noexcept : r(v.r), g(v.g), b(v.b), a(v.a) {}
    /** @brief Defaults constructs a pure black color. */
    constexpr Rgba() noexcept : r(0), g(0), b(0), a(1) {}
    /** @brief Constructs a new RGBA with the provided value for each channel, but alpha as 1.0f. */
    constexpr explicit Rgba(const f32 val) noexcept : r(val), g(val), b(val), a(1) {}
    /** @brief Constructs a new RGBA with the provided value for each color channel and using the given alpha. */
    constexpr Rgba(const f32 color, const f32 alpha) noexcept : r(color), g(color), b(color), a(alpha) {}
    /** @brief Constructs a new RGBA color provided the given values. */
    constexpr Rgba(const f32 r, const f32 g, const f32 b, const f32 a) noexcept : r(r), g(g), b(b), a(a) {}

    Rgba(const Rgba&) = default;

    Rgba(Rgba&&) = default;

    Rgba& operator=(const Rgba&) = default;

    Rgba& operator=(Rgba&&) = default;

    /** @brief Formats this RGBA as a string. */
    [[nodiscard]] auto to_string() const -> std::string { return std::format("Rgba({}, {}, {}, {})", r, g, b, a); }

    /** @brief Alias for to_glm() for better integration with vector-heavy code. */
    [[nodiscard]] constexpr auto to_vec4() const noexcept -> glm::vec4 { return glm::vec4{r, g, b, a}; }

    /** @brief Equality comparison of two RGBA values. */
    [[nodiscard]] auto operator==(const Rgba&) const noexcept -> bool = default;

    /** @brief Component wise addition. */
    [[nodiscard]] constexpr auto operator+(const Rgba& o) const noexcept -> Rgba {
        return {r + o.r, g + o.g, b + o.b, a + o.a};
    }

    /** @brief Component wise subtraction. */
    [[nodiscard]] constexpr auto operator-(const Rgba& o) const noexcept -> Rgba {
        return {r - o.r, g - o.g, b - o.b, a - o.a};
    }

    /** @brief Component wise multiplication. */
    [[nodiscard]] constexpr auto operator*(const Rgba& o) const noexcept -> Rgba {
        return {r * o.r, g * o.g, b * o.b, a * o.a};
    }

    /** @brief Scalar multiplication. */
    [[nodiscard]] constexpr auto operator*(const f32 s) const noexcept -> Rgba {
        return {r * s, g * s, b * s, a * s};
    }

    /** @brief Scalar division. */
    [[nodiscard]] constexpr auto operator/(const f32 s) const noexcept -> Rgba {
        return {r / s, g / s, b / s, a / s};
    }

    /** @brief Adds two RGBA values. */
    [[nodiscard]] constexpr auto operator+=(const Rgba& o) noexcept -> Rgba& {
        *this = *this + o;
        return *this;
    }

    /** @brief Subtracts two RGBA values. */
    constexpr auto operator-=(const Rgba& o) noexcept -> Rgba& {
        *this = *this - o;
        return *this;
    }

    /** @brief Multiplies two RGBA values. */
    constexpr auto operator*=(const Rgba& o) noexcept -> Rgba& {
        *this = *this * o;
        return *this;
    }

    /** @brief Multiplies an RGBA with a scalar. */
    constexpr auto operator*=(const f32 s) noexcept -> Rgba& {
        *this = *this * s;
        return *this;
    }

    /** @brief Divides an RGBA with a scalar. */
    constexpr auto operator/=(const f32 s) noexcept -> Rgba& {
        *this = *this / s;
        return *this;
    }

    /** @brief All values set to 0. */
    static const Rgba ZERO;
    /** @brief All values set to 1. */
    static const Rgba ONE;
    /** @brief A pure black color. */
    static const Rgba BLACK;
    /** @brief A gray color. */
    static const Rgba GRAY;
    /** @brief A pure white color. */
    static const Rgba WHITE;
    /** @brief A pure red color. */
    static const Rgba RED;
    /** @brief A pure green color. */
    static const Rgba GREEN;
    /** @brief A pure blue color. */
    static const Rgba BLUE;

};

inline const Rgba Rgba::ZERO{0.f, 0.f, 0.f, 0.f};
inline const Rgba Rgba::ONE{1.f, 1.f, 1.f, 1.f};
inline const Rgba Rgba::BLACK{0.f, 0.f, 0.f, 1.f};
inline const Rgba Rgba::GRAY{0.5f, 0.5f, 0.5f, 1.f};
inline const Rgba Rgba::WHITE{1.f, 1.f, 1.f, 1.f};
inline const Rgba Rgba::RED{1.f, 0.f, 0.f, 1.f};
inline const Rgba Rgba::GREEN{0.f, 1.f, 0.f, 1.f};
inline const Rgba Rgba::BLUE{0.f, 0.f, 1.f, 1.f};
} // namespace siren
