#pragma once

#include <array>
#include <cmath>
#include <format>
#include <span>
#include <type_traits>

#include "2iREN/base.hpp"
#include "2iREN/math/angle.hpp"
#include "2iREN/math/bounded.hpp"
#include "2iREN/math/vec3.hpp"

namespace siren {

template <typename T>
struct Mat4x4;

template <typename T>
[[nodiscard]]
constexpr auto operator*(const Mat4x4<T>& left, const Mat4x4<T>& right) noexcept -> Mat4x4<T>;

using Mat4x4f = Mat4x4<f32>;
using Mat4x4i = Mat4x4<i32>;
using Mat4x4u = Mat4x4<u32>;

/// @brief A 4x4 Dimensional Matrix. Stored column major to mirror standard
/// OpenGL layout.
///
/// This means we store elements in memory like:
///     0  4  8  12
///     1  5  9  13
///     2  6  10 14
///     3  7  11 15
template <typename T>
struct Mat4x4 {
    using Type     = std::remove_cvref_t<T>;
    using Elements = std::array<Type, 16>;
    using Column   = std::span<Type, 4>;

    Elements elements;

    constexpr explicit Mat4x4() : Mat4x4(IDENTITY()) {}
    constexpr explicit Mat4x4(Type value) :
        elements(
            {value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value,
             value}
        ) {}
    constexpr explicit Mat4x4(const Elements& elements) : elements(elements) {}
    constexpr explicit Mat4x4(const Column c0, const Column c1, const Column c2, const Column c3) :
        elements(
            {c0[0],
             c0[1],
             c0[2],
             c0[3],
             c1[0],
             c1[1],
             c1[2],
             c1[3],
             c2[0],
             c2[1],
             c2[2],
             c2[3],
             c3[0],
             c3[1],
             c3[2],
             c3[3]}
        ) {}

    [[nodiscard]]
    static constexpr auto make(const Type* ptr) -> Mat4x4 {
        return Mat4x4{Elements{
            ptr[0],
            ptr[1],
            ptr[2],
            ptr[3],
            ptr[4],
            ptr[5],
            ptr[6],
            ptr[7],
            ptr[8],
            ptr[9],
            ptr[10],
            ptr[11],
            ptr[12],
            ptr[13],
            ptr[14],
            ptr[15],
        }};
    }

    [[nodiscard]]
    static constexpr auto IDENTITY() noexcept -> Mat4x4 {
        return Mat4x4{
            {Type{1},
             Type{0},
             Type{0},
             Type{0},
             Type{0},
             Type{1},
             Type{0},
             Type{0},
             Type{0},
             Type{0},
             Type{1},
             Type{0},
             Type{0},
             Type{0},
             Type{0},
             Type{1}},
        };
    }
    [[nodiscard]]
    static constexpr auto ZERO() noexcept -> Mat4x4 {
        return Mat4x4{Type{0}};
    }

    /// @brief Rotates the given matrix around the given axis by the provided
    /// amount in radians.
    [[nodiscard]]
    static constexpr auto rotate(
        const Mat4x4& matrix,
        const Radians radians,
        const Vec3<T>& axis
    ) noexcept -> Mat4x4 {
        const auto axislen = axis.length();
        ASSERT(axislen != 0, "cannot rotate around a zero-length axis");

        const auto x = static_cast<Type>(static_cast<f64>(axis.x) / axislen);
        const auto y = static_cast<Type>(static_cast<f64>(axis.y) / axislen);
        const auto z = static_cast<Type>(static_cast<f64>(axis.z) / axislen);
        const auto c = static_cast<Type>(std::cos(radians.value));
        const auto s = static_cast<Type>(std::sin(radians.value));
        const auto t = Type{1} - c;

        const auto rotation = Mat4x4{Elements{
            t * x * x + c,
            t * x * y + s * z,
            t * x * z - s * y,
            Type{0},
            t * x * y - s * z,
            t * y * y + c,
            t * y * z + s * x,
            Type{0},
            t * x * z + s * y,
            t * y * z - s * x,
            t * z * z + c,
            Type{0},
            Type{0},
            Type{0},
            Type{0},
            Type{1},
        }};

        return matrix * rotation;
    }

    [[nodiscard]]
    static constexpr auto perspective(
        const Radians radians,
        const NonZeroPositiveF32 aspect_ratio,
        const NonZeroPositiveF32 nearplane,
        const NonZeroPositiveF32 farplane
    ) noexcept -> Mat4x4 {
        const auto halftan = static_cast<Type>(std::tan(radians.value / 2));
        const auto aspect  = static_cast<Type>(aspect_ratio.get());
        const auto near    = static_cast<Type>(nearplane.get());
        const auto far     = static_cast<Type>(farplane.get());
        ASSERT(far > near, "far plane must be greater than near plane");

        auto mat = Mat4x4::ZERO();

        mat[0][0] = Type{1} / (aspect * halftan);
        mat[1][1] = Type{1} / halftan;
        mat[2][2] = -(far + near) / (far - near);
        mat[2][3] = Type{-1};
        mat[3][2] = -(Type{2} * far * near) / (far - near);

        return mat;
    }

    [[nodiscard]]
    static constexpr auto translate(const Mat4x4& matrix, const Vec3<T>& amount) noexcept
        -> Mat4x4 {
        // clang-format off
        const Mat4x4 translation{Elements{
            Type{1}, Type{0}, Type{0}, Type{0},
            Type{0}, Type{1}, Type{0}, Type{0},
            Type{0}, Type{0}, Type{1}, Type{0},
            amount.x, amount.y, amount.z, Type{1},
        }};
        // clang-format on

        return matrix * translation;
    }

    /// @brief Scales the given matrix by the provided vector.
    [[nodiscard]]
    static constexpr auto scale(const Mat4x4& matrix, const Vec3<T> scale) noexcept -> Mat4x4 {
        const Mat4x4 scaling{Elements{
            scale.x,
            Type{0},
            Type{0},
            Type{0},
            Type{0},
            scale.y,
            Type{0},
            Type{0},
            Type{0},
            Type{0},
            scale.z,
            Type{0},
            Type{0},
            Type{0},
            Type{0},
            Type{1},
        }};

        return matrix * scaling;
    }

    [[nodiscard]]
    constexpr auto operator==(const Mat4x4&) const noexcept -> bool;

    [[nodiscard]]
    constexpr auto operator!=(const Mat4x4&) const noexcept -> bool;

    [[nodiscard]]
    constexpr auto operator[](const usize column) noexcept -> std::span<Type, 4> {
        ASSERT(column < 4, "cannot index Mat4 with value {}", column);
        return std::span<Type, 4>{elements.data() + column * 4, 4};
    }

    [[nodiscard]]
    constexpr auto operator[](const usize column) const noexcept -> std::span<const Type, 4> {
        ASSERT(column < 4, "cannot index Mat4 with value {}", column);
        return std::span<const Type, 4>{elements.data() + column * 4, 4};
    }

    [[nodiscard]]
    constexpr auto data() const -> const Type*;

    [[nodiscard]]
    constexpr auto data() -> Type*;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;
};

template <typename T>
constexpr auto operator*(const Mat4x4<T>& left, const Mat4x4<T>& right) noexcept -> Mat4x4<T> {
    auto result = Mat4x4<T>::ZERO();

    for (usize column = 0; column < 4; ++column) {
        for (usize row = 0; row < 4; ++row) {
            for (usize index = 0; index < 4; ++index) {
                result[column][row] += left[index][row] * right[column][index];
            }
        }
    }

    return result;
}

template <typename T>
constexpr auto Mat4x4<T>::operator==(const Mat4x4& other) const noexcept -> bool {
    return elements == other.elements;
}

template <typename T>
constexpr auto Mat4x4<T>::operator!=(const Mat4x4& other) const noexcept -> bool {
    return elements != other.elements;
}

template <typename T>
constexpr auto Mat4x4<T>::data() const -> const Mat4x4<T>::Type* {
    return elements.data();
}

template <typename T>
constexpr auto Mat4x4<T>::data() -> Mat4x4<T>::Type* {
    return elements.data();
}

template <typename T>
constexpr auto Mat4x4<T>::to_string() const -> std::string {
    return std::format(
        "Mat4x4<{}>(\n{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n{} {} {} {}\n )",
        typename_of<T>(),
        (*this)[0][0],
        (*this)[1][0],
        (*this)[2][0],
        (*this)[3][0],
        (*this)[0][1],
        (*this)[1][1],
        (*this)[2][1],
        (*this)[3][1],
        (*this)[0][2],
        (*this)[1][2],
        (*this)[2][2],
        (*this)[3][2],
        (*this)[0][3],
        (*this)[1][3],
        (*this)[2][3],
        (*this)[3][3]
    );
}

} // namespace siren
