#pragma once

#include <string>
#include <format>

#define DEFINE_RGBA(name, r, g, b, a) \
    [[nodiscard]] static constexpr auto name() noexcept -> RGBA { return {r, g, b, a}; }


namespace siren {

/**
 * @struct RGBA
 * @brief A four channel color.
 */
struct RGBA {
    /** @brief The red color channel. */
    float r;
    /** @brief The green color channel. */
    float g;
    /** @brief The blue color channel. */
    float b;
    /** @brief The alpha channel. */
    float a;

    /** @brief Construct from an existing GLM vector. */
    constexpr explicit RGBA(const glm::vec4& v) noexcept : r(v.r), g(v.g), b(v.b), a(v.a) { }
    /** @brief Defaults constructs a pure black color. */
    constexpr RGBA() noexcept : r(0), g(0), b(0), a(1.0f) { }
    /** @brief Constructs a new RGBA with the provided value for each channel, but alpha as 1.0f. */
    constexpr explicit RGBA(const float val) noexcept : r(val), g(val), b(val), a(1.0f) { }
    /** @brief Constructs a new RGBA with the provided value for each color channel and using the given alpha. */
    constexpr RGBA(const float color, const float alpha) noexcept : r(color), g(color), b(color), a(alpha) { }
    /** @brief Constructs a new RGBA color provided the given values. */
    constexpr RGBA(
        const float r_,
        const float g_,
        const float b_,
        const float a_
    ) noexcept : r(r_), g(g_), b(b_), a(a_) { }

    /** @brief Formats this RGBA as a string. */
    [[nodiscard]] auto to_string() -> std::string {
        return std::format("RGBA({}, {}, {}, {})", r, g, b, a);
    }

    /** @brief Alias for to_glm() for better integration with vector-heavy code. */
    [[nodiscard]] constexpr auto to_vec4() const noexcept -> glm::vec4 {
        return glm::vec4{ r, g, b, a };
    }

    /** @brief Equality comparison of two RGBA values. */
    [[nodiscard]] auto operator==(const RGBA&) const noexcept -> bool = default;

    /** @brief Component wise addition. */
    [[nodiscard]] constexpr auto operator+(const RGBA& o) const noexcept -> RGBA {
        return { r + o.r, g + o.g, b + o.b, a + o.a };
    }
    /** @brief Component wise subtraction. */
    [[nodiscard]] constexpr auto operator-(const RGBA& o) const noexcept -> RGBA {
        return { r - o.r, g - o.g, b - o.b, a - o.a };
    }
    /** @brief Component wise multiplication. */
    [[nodiscard]] constexpr auto operator*(const RGBA& o) const noexcept -> RGBA {
        return { r * o.r, g * o.g, b * o.b, a * o.a };
    }
    /** @brief Scalar multiplication. */
    [[nodiscard]] constexpr auto operator*(const float s) const noexcept -> RGBA {
        return { r * s, g * s, b * s, a * s };
    }
    /** @brief Scalar division. */
    [[nodiscard]] constexpr auto operator/(const float s) const noexcept -> RGBA {
        return { r / s, g / s, b / s, a / s };
    }
    /** @brief Adds two RGBA values. */
    [[nodiscard]] constexpr auto operator+=(const RGBA& o) noexcept -> RGBA& {
        *this = *this + o;
        return *this;
    }
    /** @brief Subtracts two RGBA values. */
    constexpr auto operator-=(const RGBA& o) noexcept -> RGBA& {
        *this = *this - o;
        return *this;
    }
    /** @brief Multiplies two RGBA values. */
    constexpr auto operator*=(const RGBA& o) noexcept -> RGBA& {
        *this = *this * o;
        return *this;
    }
    /** @brief Multiplies an RGBA with a scalar. */
    constexpr auto operator*=(const float s) noexcept -> RGBA& {
        *this = *this * s;
        return *this;
    }
    /** @brief Divides an RGBA with a scalar. */
    constexpr auto operator/=(const float s) noexcept -> RGBA& {
        *this = *this / s;
        return *this;
    }

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
