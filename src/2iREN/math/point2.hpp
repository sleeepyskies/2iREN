#pragma once

#include <format>

#include "2iREN/core/base.hpp"
#include "2iREN/utility/type_info.hpp"

namespace siren {

template <typename T>
struct Point2;

using Point2f = Point2<f32>;
using Point2u = Point2<u32>;
using Point2i = Point2<i32>;

/// @brief A three dimensional point.
template <typename T>
struct Point2 {
    using Type = std::remove_cvref_t<T>;
    T x, y;

    constexpr Point2() : Point2(T{}) { }
    constexpr Point2(T xy) : x(xy), y(xy) { }
    constexpr Point2(T x, T y) : x(x), y(y) { }

    [[nodiscard]]
    static constexpr inline auto ORIGIN() noexcept -> Point2 {
        return Point2{0};
    }

    [[nodiscard]]
    constexpr auto operator==(const Point2&) const noexcept -> bool = default;
    [[nodiscard]]
    constexpr auto operator!=(const Point2&) const noexcept -> bool = default;

    [[nodiscard]]
    constexpr auto to_string() const -> std::string;
};

static_assert(Point2f{0} == Point2f::ORIGIN());
static_assert(Point2f{2} != Point2f::ORIGIN());
static_assert(Point2f{0} == Point2f{0});
static_assert(Point2i{0} == Point2i{0});
static_assert(Point2u{0} == Point2u{0});

template <typename T>
constexpr auto Point2<T>::to_string() const -> std::string {
    return std::format("Point2<{}>(x={}, y={})", typename_of<T>(), x, y);
}

} // namespace siren
