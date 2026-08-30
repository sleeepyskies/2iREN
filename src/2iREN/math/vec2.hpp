#pragma once

#include "2iREN/base.hpp"
namespace siren {

template <typename T>
struct Vector2 {
    T x;
    T y;

    constexpr explicit Vector2() : x({}), y({}) {}
    constexpr explicit Vector2(const T val) : x(val), y(val) {}
    constexpr explicit Vector2(const T x, const T y) : x(x), y(y) {}
};

using Vector2f = Vector2<f32>;
using Vector2i = Vector2<i32>;
using Vector2u = Vector2<u32>;

} // namespace siren
