#pragma once

#include "2iREN/base.hpp"
#include "2iREN/math/vec3.hpp"

namespace siren {

template <typename T>
struct Point3;

using Point3f = Point3<f32>;
using Point3u = Point3<u32>;
using Point3i = Point3<i32>;

/// @brief A three dimensional point.
template <typename T>
struct Point3 {
    using Type = std::remove_cvref_t<T>;
    T x, y, z;

    constexpr Point3() : Point3(T{}) {}
    constexpr Point3(T xyz) : x(xyz), y(xyz), z(xyz) {}
    constexpr Point3(T x, T y, T z) : x(x), y(y), z(z) {}

    [[nodiscard]]
    static constexpr inline auto ORIGIN() noexcept -> Point3 {
        return Point3{0};
    }

    [[nodiscard]]
    static constexpr inline auto translate(const Point3 point, const Vec3<T>& translation) noexcept
        -> Point3 {
        return Point3{
            point.x + translation.x,
            point.y + translation.y,
            point.z + translation.z,
        };
    }

    [[nodiscard]]
    constexpr auto operator==(const Point3&) const noexcept -> bool = default;
    [[nodiscard]]
    constexpr auto operator!=(const Point3&) const noexcept -> bool = default;
};

/// @brief Returns a Vec3 that goes from a to b.
template <typename T>
constexpr auto operator-(const Point3<T> a, const Point3<T> b) noexcept -> Vec3<T> {
    return Vec3<T>{
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
}

static_assert(Point3f{0} == Point3f::ORIGIN());
static_assert(Point3f{2} != Point3f::ORIGIN());
static_assert(Point3f{0} == Point3f{0});
static_assert(Point3i{0} == Point3i{0});
static_assert(Point3u{0} == Point3u{0});
static_assert(std::is_same_v<decltype(Point3f{} - Point3f{}), Vec3f>);

} // namespace siren
