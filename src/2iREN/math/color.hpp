#pragma once

#include <format>

#include "2iREN/base.hpp"

namespace siren {

/// @brief A struct holding 4 color channels.
struct Rgba {
    f32 r, g, b, a;

    /** @brief Constructs a pure black color. */
    constexpr Rgba() noexcept : r(0), g(0), b(0), a(1) {}
    /** @brief Constructs a new RGBA with the provided value for each channel. */
    constexpr explicit Rgba(const f32 rgb) noexcept : r(rgb), g(rgb), b(rgb), a(1.f) {}
    /** @brief Constructs a new RGBA with the provided value for each color channel and using the
     * given alpha. */
    constexpr Rgba(const f32 rgb, const f32 alpha) noexcept : r(rgb), g(rgb), b(rgb), a(alpha) {}
    /** @brief Constructs a new RGBA color provided the given values. */
    constexpr Rgba(const f32 r, const f32 g, const f32 b, const f32 a) noexcept :
        r(r), g(g), b(b), a(a) {}

    [[nodiscard]]
    auto operator==(const Rgba&) const noexcept -> bool = default;

    /** @brief Formats this RGBA as a string. */
    [[nodiscard]]
    constexpr auto to_string() const -> std::string;

    /** @brief Constructs a new Rgba from a c style float pointer. */
    [[nodiscard]]
    static constexpr auto from_data(const f32* data) -> Rgba {
        return Rgba{data[0], data[1], data[2], data[3]};
    }

    [[nodiscard]]
    static constexpr auto ZERO() noexcept -> Rgba {
        return Rgba{0.f, 0.f, 0.f, 0.f};
    }
    [[nodiscard]]
    static constexpr auto ONE() noexcept -> Rgba {
        return Rgba{1.f};
    }
    [[nodiscard]]
    static constexpr auto BLACK() noexcept -> Rgba {
        return Rgba{};
    }
    [[nodiscard]]
    static constexpr auto GRAY() noexcept -> Rgba {
        return Rgba{0.5f, 1.f};
    }
    [[nodiscard]]
    static constexpr auto WHITE() noexcept -> Rgba {
        return Rgba{1.f, 1.f};
    }
    [[nodiscard]]
    static constexpr auto RED() noexcept -> Rgba {
        return Rgba{1.f, 0.f, 0.f, 1.f};
    }
    [[nodiscard]]
    static constexpr auto GREEN() noexcept -> Rgba {
        return Rgba{0.f, 1.f, 0.f, 1.f};
    }
    [[nodiscard]]
    static constexpr auto BLUE() noexcept -> Rgba {
        return Rgba{0.f, 0.f, 1.f, 1.f};
    }

    // TODO: can we just return addr of r here? idts

    [[nodiscard]]
    constexpr auto data() const noexcept -> const f32* {
        return &r;
    }

    [[nodiscard]]
    constexpr auto data() noexcept -> f32* {
        return &r;
    }
};

constexpr auto Rgba::to_string() const -> std::string {
    return std::format("Rgba({}, {}, {}, {})", r, g, b, a);
}

} // namespace siren
