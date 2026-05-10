#pragma once

#include <stdfloat>
#include <cstdint>
#include <cstddef>
#include <ranges>


namespace siren {

namespace ranges = std::ranges;
namespace views = std::ranges::views;

/** @brief Toggles single threaded mode in siren. */
inline constexpr bool single_threaded = true;

/** @brief An unsigned 8-bit integer, aka a byte */
using u8 = std::uint8_t;
/** @brief An unsigned 16-bit integer */
using u16 = std::uint16_t;
/** @brief An unsigned 32-bit integer */
using u32 = std::uint32_t;
/** @brief An unsigned 64-bit integer */
using u64 = std::uint64_t;
/** @brief An unsigned 128-bit integer */
using u128 = __uint128_t;

/** @brief A signed 8-bit integer */
using i8 = std::int8_t;
/** @brief A signed 16-bit integer */
using i16 = std::int16_t;
/** @brief A signed 32-bit integer */
using i32 = std::int32_t;
/** @brief A signed 64-bit integer */
using i64 = std::int64_t;
/** @brief A signed 128-bit integer */
using i128 = __int128_t;

/** @brief A 16-bit floating point number. */
using f16 = std::float16_t;
/** @brief A 32-bit floating point number. */
using f32 = std::float32_t;
/** @brief A 64-bit floating point number. */
using f64 = std::float64_t;
/** @brief A 128-bit floating point number. */
using f128 = std::float128_t;

/** @brief An unsigned integer capable of holding the largest memory address. */
using usize = std::size_t;

/** @brief @todo do this. */
#define bit(x) (1u << (x))

} // namespace siren
