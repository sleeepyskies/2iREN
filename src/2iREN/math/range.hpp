#pragma once

#include "2iREN/core/base.hpp"

namespace siren {

/// @brief Represents an inlusive range between an inlusive start and an
/// exclusive end.
template <typename T>
struct Range {
    /// @brief The inlusive begin.
    T begin;
    /// @brief The exclusive end.
    T end;

    /// @brief Returns the length of the range.
    constexpr auto length() const noexcept -> T {
        return end - begin;
    }
};

using RangeF32   = Range<f32>;
using RangeI32   = Range<i32>;
using RangeU32   = Range<u32>;
using RangeUsize = Range<usize>;

} // namespace siren
