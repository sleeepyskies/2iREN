#pragma once

#include <cmath>
#include <format>
#include <limits>
#include <string>
#include <type_traits>

#include "2iREN/base.hpp"
#include "2iREN/core/assert.hpp"
#include "2iREN/utility/type_info.hpp"

namespace siren {

template <typename T>
struct Vec3;

using Vec3f = Vec3<f32>;
using Vec3d = Vec3<f64>;
using Vec3i = Vec3<i32>;
using Vec3u = Vec3<u32>;
using Vec3s = Vec3<std::string>;

template <typename T>
struct Vec3 {
    using Type = std::remove_cvref_t<T>;
    Type x, y, z;

    constexpr Vec3() : Vec3(Type{}) {}
    constexpr explicit Vec3(const Type xyz) : x(xyz), y(xyz), z(xyz) {}
    constexpr Vec3(const Type x, const Type y, const Type z) : x(x), y(y), z(z) {}

    template <typename S>
        requires(std::is_convertible_v<S, Type>)
    constexpr explicit Vec3(const S xyz) : Vec3(static_cast<Type>(xyz)) {}

    template <typename S>
        requires(std::is_convertible_v<S, Type>)
    constexpr Vec3(const S x, const S y, const S z) :
        Vec3(static_cast<Type>(x), static_cast<Type>(y), static_cast<Type>(z)) {}

    [[nodiscard]]
    static constexpr auto make(const Type* ptr) -> Vec3 {
        ASSERT(ptr != nullptr, "cannot create Vec3 from nullptr");
        return Vec3{ptr[0], ptr[1], ptr[2]};
    }

    [[nodiscard]]
    static constexpr auto ZERO() noexcept -> Vec3 {
        return Vec3{Type{0}};
    }
    [[nodiscard]]
    static constexpr auto ONE() noexcept -> Vec3 {
        return Vec3{Type{1}};
    }
    [[nodiscard]]
    static constexpr auto NEGATIVE_ONE() noexcept -> Vec3
        requires(std::is_signed_v<Type> || std::is_floating_point_v<Type>)
    {
        return Vec3{Type{-1}};
    }
    [[nodiscard]]
    static constexpr auto MIN() noexcept -> Vec3 {
        return Vec3{std::numeric_limits<Type>::lowest()};
    }
    [[nodiscard]]
    static constexpr auto MAX() noexcept -> Vec3 {
        return Vec3{std::numeric_limits<Type>::max()};
    }
    [[nodiscard]]
    static constexpr auto UP() noexcept -> Vec3 {
        return Vec3{0, 1, 0};
    }
    [[nodiscard]]
    static constexpr auto RIGHT() noexcept -> Vec3 {
        return Vec3{1, 0, 0};
    }

    [[nodiscard]]
    constexpr auto length() const -> f64
        requires(std::is_arithmetic_v<Type>);

    /// @brief Normalizes a vector such that vec.length() == 1.
    [[nodiscard]]
    static constexpr auto normalize(const Vec3& vec) -> Vec3
        requires(std::is_floating_point_v<Type>)
    {
        const auto length = vec.length();
        if (length == 0) {
            return vec;
        }
        return Vec3{
            static_cast<Type>(vec.x / length),
            static_cast<Type>(vec.y / length),
            static_cast<Type>(vec.z / length),
        };
    }

    /// @brief Calculcates the cross product of two vectors. This produces
    /// a new vector that is perpendicular to both input vectors.
    [[nodiscard]]
    static constexpr auto cross(const Vec3& left, const Vec3& right) -> Vec3 {
        return Vec3{
            left.y * right.z - left.z * right.y,
            left.z * right.x - left.x * right.z,
            left.x * right.y - left.y * right.x,
        };
    }

    /// @brief Returns the dot product of two vectors. This is how much they
    /// align in their direction.
    [[nodiscard]]
    static constexpr auto dot(const Vec3& left, const Vec3& right) -> T {
        return (left.x * right.x) + (left.y * right.y) + (left.z * right.z);
    }

    [[nodiscard]]
    constexpr auto operator==(const Vec3&) const noexcept -> bool = default;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;
};

template <typename T>
constexpr auto operator-=(Vec3<T>& left, Vec3<T> right) -> Vec3<T>& {
    left.x -= right.x;
    left.y -= right.y;
    left.z -= right.z;
    return left;
}

template <typename T>
[[nodiscard]]
constexpr auto operator-(Vec3<T> left, Vec3<T> right) noexcept -> Vec3<T> {
    left -= right;
    return left;
}

template <typename T>
constexpr auto operator+=(Vec3<T>& left, Vec3<T> right) -> Vec3<T>& {
    left.x += right.x;
    left.y += right.y;
    left.z += right.z;
    return left;
}

template <typename T>
[[nodiscard]]
constexpr auto operator+(Vec3<T> left, Vec3<T> right) noexcept -> Vec3<T> {
    left += right;
    return left;
}

template <typename T>
[[nodiscard]]
constexpr auto operator-(Vec3<T> vec) noexcept -> Vec3<T> {
    return Vec3<T>{} - vec;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
[[nodiscard]]
constexpr auto operator-(Vec3<T> left, S right) noexcept -> Vec3<T> {
    left.x -= right;
    left.y -= right;
    left.z -= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
[[nodiscard]]
constexpr auto operator+(Vec3<T> left, S right) noexcept -> Vec3<T> {
    left.x += right;
    left.y += right;
    left.z += right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
constexpr auto operator-=(Vec3<T>& left, S right) noexcept -> Vec3<T>& {
    left.x -= right;
    left.y -= right;
    left.z -= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
constexpr auto operator+=(Vec3<T>& left, S right) noexcept -> Vec3<T>& {
    left.x += right;
    left.y += right;
    left.z += right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
constexpr auto operator*=(Vec3<T>& left, S right) noexcept -> Vec3<T>& {
    left.x *= right;
    left.y *= right;
    left.z *= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
constexpr auto operator/=(Vec3<T>& left, S right) noexcept -> Vec3<T>& {
    ASSERT(right != 0, "cannot divide by zero.");
    left.x /= right;
    left.y /= right;
    left.z /= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
[[nodiscard]]
constexpr auto operator*(Vec3<T> left, S right) noexcept -> Vec3<T> {
    left *= right;
    return left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
[[nodiscard]]
constexpr auto operator*(S left, Vec3<T> right) noexcept -> Vec3<T> {
    return right * left;
}

template <typename T, typename S>
    requires(std::is_convertible_v<S, typename Vec3<T>::Type>)
[[nodiscard]]
constexpr auto operator/(Vec3<T> left, S right) noexcept -> Vec3<T> {
    left /= right;
    return left;
}

template <typename T>
constexpr auto Vec3<T>::length() const -> f64
    requires(std::is_arithmetic_v<Type>)
{
    const auto x_value = static_cast<f64>(x);
    const auto y_value = static_cast<f64>(y);
    const auto z_value = static_cast<f64>(z);
    return std::sqrt(x_value * x_value + y_value * y_value + z_value * z_value);
}

template <typename T>
constexpr auto Vec3<T>::to_string() const -> std::string {
    return std::format("Vec3<{}>(x={}, y={}, z={})", typename_of<T>(), x, y, z);
}

} // namespace siren
