#pragma once

#include <format>
#include <string>

#include "2iREN/core/base.hpp"
#include "2iREN/utility/concepts.hpp"

namespace siren {

struct Extent3;

struct Extent2 {
    u32 x, y;

    constexpr Extent2() = default;

    template <CanConvert<u32> T>
    constexpr explicit Extent2(const T val) : Extent2(val, val) { }

    template <CanConvert<u32> X, CanConvert<u32> Y>
    constexpr explicit Extent2(const X x, const Y y) :
        x(static_cast<u32>(x)), y(static_cast<u32>(y)) { }

    [[nodiscard]]
    constexpr auto operator==(const Extent2&) const noexcept -> bool = default;

    [[nodiscard]]
    constexpr auto to_extent3() const -> Extent3;

    [[nodiscard]]
    auto to_string() const -> std::string;

    [[nodiscard]]
    constexpr auto area() const -> usize;
};

struct Extent3 {
    u32 x, y, z;

    constexpr Extent3() = default;

    template <CanConvert<u32> T>
    constexpr explicit Extent3(const T val) : Extent3(val, val, val) { }

    template <CanConvert<u32> X, CanConvert<u32> Y, CanConvert<u32> Z>
    constexpr explicit Extent3(const X x, const Y y, const Z z) :
        x(static_cast<u32>(x)), y(static_cast<u32>(y)), z(static_cast<u32>(z)) { }

    [[nodiscard]]
    constexpr auto operator==(const Extent3&) const noexcept -> bool = default;

    [[nodiscard]]
    constexpr auto to_extent2() const -> Extent2;

    [[nodiscard]]
    auto to_string() const -> std::string;

    [[nodiscard]]
    constexpr auto volume() const -> usize;
};

constexpr auto Extent2::to_extent3() const -> Extent3 {
    return Extent3{x, y, 1u};
}

inline auto Extent2::to_string() const -> std::string {
    return std::format("Extent2(x={}, y={})", x, y);
}

constexpr auto Extent2::area() const -> usize {
    return static_cast<usize>(x) * y;
}

constexpr auto Extent3::to_extent2() const -> Extent2 {
    return Extent2{x, y};
}

inline auto Extent3::to_string() const -> std::string {
    return std::format("Extent3(x={}, y={}, z={})", x, y, z);
}

constexpr auto Extent3::volume() const -> usize {
    return x * y * z;
}

} // namespace siren
