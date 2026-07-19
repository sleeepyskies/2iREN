#pragma once

#include <format>
#include <glm/glm.hpp>
#include <string>

#define DEFINE_RGBA(name, r, g, b, a)                                                                                  \
    [[nodiscard]] static constexpr auto name() noexcept -> Rgba { return {r, g, b, a}; }

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
    constexpr explicit Rgba(const float val) noexcept : r(val), g(val), b(val), a(1) {}
    /** @brief Constructs a new RGBA with the provided value for each color channel and using the given alpha. */
    constexpr Rgba(const float color, const float alpha) noexcept : r(color), g(color), b(color), a(alpha) {}
    /** @brief Constructs a new RGBA color provided the given values. */
    constexpr Rgba(const float r, const float g, const float b, const float a) noexcept : r(r), g(g), b(b), a(a) {}

    Rgba(const Rgba&)            = default;
    Rgba(Rgba&&)                 = default;
    Rgba& operator=(const Rgba&) = default;
    Rgba& operator=(Rgba&&)      = default;

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
    [[nodiscard]] constexpr auto operator*(const float s) const noexcept -> Rgba {
        return {r * s, g * s, b * s, a * s};
    }
    /** @brief Scalar division. */
    [[nodiscard]] constexpr auto operator/(const float s) const noexcept -> Rgba {
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
    constexpr auto operator*=(const float s) noexcept -> Rgba& {
        *this = *this * s;
        return *this;
    }
    /** @brief Divides an RGBA with a scalar. */
    constexpr auto operator/=(const float s) noexcept -> Rgba& {
        *this = *this / s;
        return *this;
    }

    /** @brief All values set to 0. */
    DEFINE_RGBA(zero, 0.f, 0.f, 0.f, 0.f)
    /** @brief All values set to 1. */
    DEFINE_RGBA(one, 1.f, 1.f, 1.f, 1.f)
    /** @brief A pure black color. */
    DEFINE_RGBA(black, 0.f, 0.f, 0.f, 1.f)
    /** @brief A gray color. */
    DEFINE_RGBA(gray, 0.5f, 0.5f, 0.5f, 1.f)
    /** @brief A pure white color. */
    DEFINE_RGBA(white, 1.f, 1.f, 1.f, 1.f)
    /** @brief A pure red color. */
    DEFINE_RGBA(red, 1.f, 0.f, 0.f, 1.f)
    /** @brief A pure green color. */
    DEFINE_RGBA(green, 0.f, 1.f, 0.f, 1.f)
    /** @brief A pure blue color. */
    DEFINE_RGBA(blue, 0.f, 0.f, 1.f, 1.f)
};

#undef DEFINE_RGBA

} // namespace siren
