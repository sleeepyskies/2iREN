#pragma once

#include <format>
#include <limits>
#include <type_traits>

#include "2iREN/base.hpp"
#include "2iREN/core/assert.hpp"
#include "2iREN/utility/type_info.hpp"

namespace siren {

template <typename T>
struct Vec2;

using Vec2f = Vec2<f32>;
using Vec2d = Vec2<f64>;
using Vec2i = Vec2<i32>;
using Vec2u = Vec2<u32>;

template <typename T>
struct Vec2 {
    using Type = std::remove_cvref_t<T>;
    Type x, y;

    constexpr Vec2() : Vec2(Type{}) {}
    constexpr explicit Vec2(const Type xy) : x(xy), y(xy) {}
    constexpr explicit Vec2(const Type x, const Type y) : x(x), y(y) {}

    template <typename S>
        requires(!std::is_same_v<std::remove_cvref_t<S>, Type> && std::is_convertible_v<S, Type>)
    constexpr explicit Vec2(const S xy) : Vec2(static_cast<Type>(xy)) {}
    template <typename S>
        requires(!std::is_same_v<std::remove_cvref_t<S>, Type> && std::is_convertible_v<S, Type>)
    constexpr explicit Vec2(const S x, const S y) :
        Vec2(static_cast<Type>(x), static_cast<Type>(y)) {}

    [[nodiscard]]
    static constexpr auto make(const Type* ptr) -> Vec2 {
        return Vec2{ptr[0], ptr[1]};
    }

    [[nodiscard]]
    static constexpr auto ZERO() noexcept -> Vec2 {
        return Vec2{Type{0}};
    }
    [[nodiscard]]
    static constexpr auto ONE() noexcept -> Vec2 {
        return Vec2{Type{1}};
    }
    [[nodiscard]]
    static constexpr auto NEGATIVE_ONE() noexcept -> Vec2
        requires(std::is_signed_v<Type> || std::is_floating_point_v<Type>)
    {
        return Vec2{Type{-1}};
    }
    [[nodiscard]]
    static constexpr auto MIN() noexcept -> Vec2 {
        return Vec2{std::numeric_limits<Type>::lowest()};
    }
    [[nodiscard]]
    static constexpr auto MAX() noexcept -> Vec2 {
        return Vec2{std::numeric_limits<Type>::max()};
    }

    [[nodiscard]]
    constexpr auto operator==(const Vec2&) const noexcept -> bool = default;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;
};

template <typename T>
constexpr auto operator-=(Vec2<T>& left, Vec2<T> right) -> Vec2<T>& {
    left.x -= right.x;
    left.y -= right.y;
    return left;
}

template <typename T>
[[nodiscard]]
constexpr auto operator-(Vec2<T> left, Vec2<T> right) noexcept -> Vec2<T> {
    left -= right;
    return left;
}

template <typename T>
constexpr auto operator+=(Vec2<T>& left, Vec2<T> right) -> Vec2<T>& {
    left.x += right.x;
    left.y += right.y;
    return left;
}

template <typename T>
[[nodiscard]]
constexpr auto operator+(Vec2<T> left, Vec2<T> right) noexcept -> Vec2<T> {
    left += right;
    return left;
}

template <typename T>
[[nodiscard]]
constexpr auto operator-(Vec2<T> vec) noexcept -> Vec2<T> {
    return Vec2<T>{} - vec;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
[[nodiscard]]
constexpr auto operator-(Vec2<T> left, S right) noexcept -> Vec2<T> {
    left.x -= right;
    left.y -= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
[[nodiscard]]
constexpr auto operator+(Vec2<T> left, S right) noexcept -> Vec2<T> {
    left.x += right;
    left.y += right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
[[nodiscard]]
constexpr auto operator*(Vec2<T> left, S right) noexcept -> Vec2<T> {
    left.x *= right;
    left.y *= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
[[nodiscard]]
constexpr auto operator*(S left, Vec2<T> right) noexcept -> Vec2<T> {
    return right * left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
[[nodiscard]]
constexpr auto operator/(Vec2<T> left, S right) noexcept -> Vec2<T> {
    ASSERT(right != 0, "cannot divide by zero.");
    left.x /= right;
    left.y /= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
constexpr auto operator+=(Vec2<T>& left, S right) noexcept -> Vec2<T>& {
    left.x += right;
    left.y += right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
constexpr auto operator-=(Vec2<T>& left, S right) noexcept -> Vec2<T>& {
    left.x -= right;
    left.y -= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
constexpr auto operator*=(Vec2<T>& left, S right) noexcept -> Vec2<T>& {
    left.x *= right;
    left.y *= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec2<T>::Type>)
constexpr auto operator/=(Vec2<T>& left, S right) noexcept -> Vec2<T>& {
    ASSERT(right != 0, "cannot divide by zero.");
    left.x /= right;
    left.y /= right;
    return left;
}

template <typename T>
constexpr auto Vec2<T>::to_string() const -> std::string {
    return std::format("Vec2<{}>(x={}, y={})", typename_of<T>(), x, y);
}

} // namespace siren
