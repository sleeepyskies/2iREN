#pragma once

#include <format>
#include <type_traits>

#include "2iREN/base.hpp"
#include "2iREN/utility/type_info.hpp"

namespace siren {

template <typename T>
struct Extent2;

template <typename T>
struct Extent3;

using Extent2f = Extent2<f32>;
using Extent2i = Extent2<i32>;
using Extent2u = Extent2<u32>;

using Extent3f = Extent3<f32>;
using Extent3i = Extent3<i32>;
using Extent3u = Extent3<u32>;

template <typename T>
struct Extent2 {
    using Type = std::remove_cvref_t<T>;
    Type x, y;

    template <typename S>
        requires(std::is_constructible_v<S, T>)
    constexpr Extent2() : Extent2(S{}) {}
    template <typename S>
        requires(std::is_constructible_v<S, T>)
    constexpr explicit Extent2(const S val) : x(val), y(val) {}
    template <typename S>
        requires(std::is_constructible_v<S, T>)
    constexpr explicit Extent2(const S x, const S y) : x(x), y(y) {}

    constexpr Extent2() : Extent2(Type{}) {}
    constexpr explicit Extent2(const Type val) : x(val), y(val) {}
    constexpr explicit Extent2(const Type x, const Type y) : x(x), y(y) {}

    [[nodiscard]]
    constexpr auto operator==(const Extent2&) const noexcept -> bool = default;
    [[nodiscard]]
    constexpr auto operator!=(const Extent2&) const noexcept -> bool = default;

    [[nodiscard]]
    constexpr auto to_extent3() const -> Extent3<T>;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;
};

template <typename T>
struct Extent3 {
    using Type = std::remove_cvref_t<T>;
    Type x, y, z;

    template <typename S>
        requires(std::is_constructible_v<S, T>)
    constexpr Extent3() : Extent3(S{}) {}
    template <typename S>
        requires(std::is_constructible_v<S, T>)
    constexpr explicit Extent3(const S val) : x(val), y(val), z(val) {}
    template <typename S>
        requires(std::is_constructible_v<S, T>)
    constexpr explicit Extent3(const S x, const S y, const S z) : x(x), y(y), z(z) {}

    constexpr Extent3() : Extent3(Type{}) {}
    constexpr explicit Extent3(const Type val) : x(val), y(val), z(val) {}
    constexpr explicit Extent3(const Type x, const Type y, const Type z) : x(x), y(y), z(z) {}

    [[nodiscard]]
    constexpr auto operator==(const Extent3&) const noexcept -> bool = default;
    [[nodiscard]]
    constexpr auto operator!=(const Extent3&) const noexcept -> bool = default;

    [[nodiscard]]
    constexpr auto to_extent2() const -> Extent2<T>;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;
};

template <typename T>
constexpr auto Extent2<T>::to_extent3() const -> Extent3<T> {
    return Extent3<T>{x, y, 1};
}

template <typename T>
constexpr auto Extent2<T>::to_string() const -> std::string {
    return std::format("Extent2<{}>(x={}, y={})", typename_of<T>(), x, y);
}

template <typename T>
constexpr auto Extent3<T>::to_extent2() const -> Extent2<T> {
    return Extent2<T>{x, y};
}

template <typename T>
constexpr auto Extent3<T>::to_string() const -> std::string {
    return std::format("Extent3<{}>(x={}, y={}, z={})", typename_of<T>(), x, y, z);
}

} // namespace siren
