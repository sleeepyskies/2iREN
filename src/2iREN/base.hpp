#pragma once

#include <stdfloat>
#include <cstdint>
#include <cstddef>
#include <ranges>

namespace siren {

#ifndef SIREN_SINGLE_THREADED
#define SIREN_SINGLE_THREADED 0
#endif

inline constexpr bool SINGLE_THREADED = SIREN_SINGLE_THREADED;


/** @brief Simple macro to mark wrapping PANIC with an unimplemented message. */
#define UNIMPLEMENTED() PANIC("Not implemented")

/**
 * @brief The backend to initialize 2iREN with.
 */
enum class Backend {
    /** @brief Let 2iREN handle selecting the best rendering API. */
    Auto,
    /** @brief Use OpenGL 4.6. */
    OpenGL,
};

/** @brief An unsigned 8-bit integer, aka a byte */
using u8 = std::uint8_t;
/** @brief An unsigned 16-bit integer */
using u16 = std::uint16_t;
/** @brief An unsigned 32-bit integer */
using u32 = std::uint32_t;
/** @brief An unsigned 64-bit integer */
using u64 = std::uint64_t;

/** @brief A signed 8-bit integer */
using i8 = std::int8_t;
/** @brief A signed 16-bit integer */
using i16 = std::int16_t;
/** @brief A signed 32-bit integer */
using i32 = std::int32_t;
/** @brief A signed 64-bit integer */
using i64 = std::int64_t;

/** @brief A 32-bit floating point number. */
using f32 = float;
/** @brief A 64-bit floating point number. */
using f64 = double;

/** @brief An unsigned integer capable of holding the largest memory address. */
using usize = std::size_t;

/**
 * @brief Creates a range.
 * @param end The exclusive end point of the range.
 * @return A range from [0, end).
 */
constexpr auto range(const usize end) { return std::views::iota(0u, end); }

/**
 * @brief Creates an iterable range.
 * @param start The inclusive start point of the range.
 * @param end The exclusive end point of the range.
 * @return A range from [start, end).
 */
constexpr auto range(const usize start, const usize end) { return std::views::iota(start, end); }

/**
 * Aligns the provided size parameter according to the alignment parameter.
 * Aka, rounds size to the nearest multiple of alignment > than size.
 */
constexpr auto align_up(const usize size, const usize alignment) -> usize {
    return (size + alignment - 1) / alignment * alignment;
}
} // namespace siren
