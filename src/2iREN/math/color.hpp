#pragma once

#include <format>
#include <string>

#include "2iREN/base.hpp"

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

    /** @brief Defaults constructs a pure black color. */
    constexpr Rgba() noexcept : r(0), g(0), b(0), a(1) {}
    /** @brief Constructs a new RGBA with the provided value for each channel. */
    constexpr explicit Rgba(const f32 val) noexcept : r(val), g(val), b(val), a(val) {}
    /** @brief Constructs a new RGBA with the provided value for each color channel and using the given alpha. */
    constexpr Rgba(const f32 val, const f32 alpha) noexcept : r(val), g(val), b(val), a(alpha) {}
    /** @brief Constructs a new RGBA color provided the given values. */
    constexpr Rgba(const f32 r, const f32 g, const f32 b, const f32 a) noexcept : r(r), g(g), b(b), a(a) {}

    Rgba(const Rgba&)            = default;
    Rgba(Rgba&&)                 = default;
    Rgba& operator=(const Rgba&) = default;
    Rgba& operator=(Rgba&&)      = default;

    [[nodiscard]] auto operator==(const Rgba&) const noexcept -> bool = default;

    /** @brief Formats this RGBA as a string. */
    [[nodiscard]] auto to_string() const -> std::string { return std::format("Rgba({}, {}, {}, {})", r, g, b, a); }

    /** @brief Returns a raw data pointer to this Rgba. */
    [[nodiscard]] auto data() const -> const f32* { return &r; }

    /** @brief Constructs a new Rgba from a c style float pointer. */
    [[nodiscard]] static constexpr auto from_data(const f32* data) -> Rgba {
        return Rgba{data[0], data[1], data[2], data[3]};
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
