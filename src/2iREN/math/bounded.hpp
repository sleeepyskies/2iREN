#pragma once

#include <concepts>
#include <limits>
#include <type_traits>

#include "2iREN/base.hpp"
#include "2iREN/core/assert.hpp"
#include "2iREN/utility/concepts.hpp"

// TODO: this is not done, need to add op overloads and check values
// TODO: add implicit conversion to underlying type?
// TODO: add assignment and constructors!

namespace siren {

struct AssertBoundsPolicy {
    template <IsComparable T, typename LowerPolicy, typename UpperPolicy>
    static constexpr auto check_bounds(T& value, const T& min, const T& max) -> void {
        ASSERT(
            LowerPolicy::check_lower(value, min) && UpperPolicy::check_upper(value, max),
            "value {} is not within bounds {}{}, {}{}.",
            value,
            LowerPolicy::LParen,
            min,
            max,
            UpperPolicy::RParen
        );
    }
};

struct ClampBoundsPolicy {
    template <IsComparable T, typename LowerPolicy, typename UpperPolicy>
    static constexpr auto check_bounds(T& value, const T& min, const T& max) -> void {
        if (!LowerPolicy::check_lower(value, min)) {
            value = min;
        } else if (!UpperPolicy::check_upper(value, max)) {
            value = max;
        }
    }
};

struct InclusiveBoundsPolicy {
    template <IsComparable T>
    static constexpr auto check_lower(const T& value, const T& min) -> bool {
        return value >= min;
    }
    template <IsComparable T>
    static constexpr auto check_upper(const T& value, const T& max) -> bool {
        return value <= max;
    }

    static constexpr std::string_view LParen = "[";
    static constexpr std::string_view RParen = "]";
};

struct ExclusiveBoundsPolicy {
    template <IsComparable T>
    static constexpr auto check_lower(const T& value, const T& min) -> bool {
        return value > min;
    }
    template <IsComparable T>
    static constexpr auto check_upper(const T& value, const T& max) -> bool {
        return value < max;
    }

    static constexpr std::string_view LParen = "(";
    static constexpr std::string_view RParen = ")";
};

/// @brief Some type that is bounded by an inclusive min and max.
template <
    IsComparable T,
    T Min,
    T Max,
    typename BoundsPolicy = AssertBoundsPolicy,
    typename LowerPolicy  = InclusiveBoundsPolicy,
    typename UpperPolicy  = InclusiveBoundsPolicy>
class Bounded;

/// @brief An f32 that is bounded by an inclusive min and max.
template <
    f32 Min               = std::numeric_limits<f32>::min(),
    f32 Max               = std::numeric_limits<f32>::max(),
    typename BoundsPolicy = AssertBoundsPolicy,
    typename LowerPolicy  = InclusiveBoundsPolicy,
    typename UpperPolicy  = InclusiveBoundsPolicy>
using BoundedF32 = Bounded<f32, Min, Max, BoundsPolicy, LowerPolicy, UpperPolicy>;

/// @brief An f32 that may not be negative, but has no upper bound.
using PositiveF32 = BoundedF32<0.f, std::numeric_limits<f32>::max(), AssertBoundsPolicy>;

/// @brief An f32 that may not be negative or 0, but has no upper bound.
using NonZeroPositiveF32 =
    BoundedF32<0.f, std::numeric_limits<f32>::max(), AssertBoundsPolicy, ExclusiveBoundsPolicy>;

/// @brief An i32 that is bounded by an inclusive min and max.
template <i32 Min = std::numeric_limits<i32>::min(), i32 Max = std::numeric_limits<i32>::max()>
using BoundedI32 = Bounded<i32, Min, Max>;

/// @brief A u32 that is bounded by an inclusive min and max.
template <u32 Min = std::numeric_limits<u32>::min(), u32 Max = std::numeric_limits<u32>::max()>
using BoundedU32 = Bounded<u32, Min, Max>;

template <
    IsComparable T,
    T Min,
    T Max,
    typename BoundsPolicy,
    typename LowerPolicy,
    typename UpperPolicy>
class Bounded {
public:
    using Type                    = std::remove_cvref_t<T>;
    inline static constexpr T MIN = Min;
    inline static constexpr T MAX = Max;

    constexpr Bounded() : Bounded(T{}) { }

    constexpr Bounded(T value) : m_value(value) {
        check_bounds();
    }

    template <typename Self>
    [[nodiscard]]
    constexpr auto get(this Self&& self) noexcept {
        return std::forward<Self>(self).m_value;
    }

    constexpr auto set(const T& value) -> void {
        m_value = value;
        check_bounds();
    }

    template <typename S, S OtherMin, S OtherMax>
        requires(CanConvertTo<std::remove_cvref_t<S>, Type>)
    [[nodiscard]]
    constexpr auto operator<=>(const Bounded<S, OtherMax, OtherMin>& other) const noexcept -> auto {
        other.m_value <=> m_value;
    }

private:
    constexpr auto check_bounds() -> void {
        BoundsPolicy::template check_bounds<Type, LowerPolicy, UpperPolicy>(m_value, MIN, MAX);
    }

    T m_value;
};

} // namespace siren
