#pragma once

#include <stdfloat>
#include <cstdint>
#include <cstddef>
#include <ranges>


namespace siren {
namespace ranges = std::ranges;
namespace views = std::ranges::views;

/** @brief Simple macro to mark wrapping PANIC with an unimplemented message. */
#define UNIMPLEMENTED() PANIC("Not implemented")

/**
 * @brief The backend to initialize 2iren with.
 */
enum class Backend {
    /** @brief Let 2iren handle selecting the best rendering API. */
    Auto,
    /** @brief Use OpenGL 4.6. */
    OpenGL,
};

/** @brief Toggles single threaded mode in siren. */
inline constexpr bool SINGLE_THREADED = true;

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

// todo: clang doesnt seem to support std::floatx_t, so we use these compiler primitives instead, not ideal though.

/** @brief A 16-bit floating point number. */
using f16 = _Float16;
/** @brief A 32-bit floating point number. */
using f32 = _Float32;
/** @brief A 64-bit floating point number. */
using f64 = _Float64;
/** @brief A 128-bit floating point number. */
using f128 = _Float128;

/** @brief An unsigned integer capable of holding the largest memory address. */
using usize = std::size_t;

/**
 * @brief Creates a range.
 * @param end The exclusive end point of the range.
 * @return A range from [0, end).
 */
constexpr auto range(const usize end) { return views::iota(0u, end); }

/**
 * @brief Creates an iterable range.
 * @param start The inclusive start point of the range.
 * @param end The exclusive end point of the range.
 * @return A range from [start, end).
 */
constexpr auto range(const usize start, const usize end) { return views::iota(start, end); }

/**
 * Aligns the provided size parameter according to the alignment parameter.
 * Aka, rounds size to the nearest multiple of alignment > than size.
 */
constexpr auto align_up(const usize size, const usize alignment) -> usize {
    return (size + alignment - 1) / alignment * alignment;
}
} // namespace siren
