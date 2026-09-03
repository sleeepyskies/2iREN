#pragma once

#include <format>
#include <string>

#include "2iREN/base.hpp"

namespace siren {

struct Radians;
struct Degrees;

struct Radians {
    constexpr explicit Radians(const f64 value) : value(value) {}

    [[nodiscard]]
    constexpr auto to_degrees() const -> Degrees;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;

    [[nodiscard]]
    constexpr auto operator<=>(const Radians&) const noexcept = default;

    f64 value;
};

struct Degrees {
    constexpr explicit Degrees(const f64 value) : value(value) {}

    [[nodiscard]]
    constexpr auto to_radians() const -> Radians;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;

    [[nodiscard]]
    constexpr auto operator<=>(const Degrees&) const noexcept = default;

    f64 value;
};

constexpr auto Radians::to_degrees() const -> Degrees {
    return Degrees{value * 180.f / std::numbers::pi};
}

constexpr auto Radians::to_string() const -> std::string {
    return std::format("Radians({})", value);
}

constexpr auto Degrees::to_radians() const -> Radians {
    return Radians{value * std::numbers::pi / 180.f};
}

constexpr auto Degrees::to_string() const -> std::string {
    return std::format("Degrees({})", value);
}

} // namespace siren
