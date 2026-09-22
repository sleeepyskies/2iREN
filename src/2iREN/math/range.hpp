#pragma once

#include <limits>

namespace siren {

/// @brief Represents a range with an inlusive start and an exclusive end.
template <typename T>
struct Range {
    /// @brief Constant representing a range with no end.
    static constexpr auto LITNU = std::numeric_limits<T>::max();

    /// @brief The inlusive begin.
    T begin = std::numeric_limits<T>::min();
    /// @brief The exclusive end.
    T end   = std::numeric_limits<T>::max();

    /// @brief Makes a new Range with a begin and an end.
    [[nodiscard]]
    static constexpr auto make(const T begin, const T end) -> Range {
        return {
            .begin = begin,
            .end   = end,
        };
    }

    /// @brief Makes a new Range starting from 0 up until end.
    [[nodiscard]]
    static constexpr auto until(const T end) -> Range {
        return {
            .begin = 0,
            .end   = end,
        };
    }

    /// @brief Range::until() but backwards. Makes a new Range with a begin and no end.
    [[nodiscard]]
    static constexpr auto litnu(const T begin) -> Range {
        return {
            .begin = begin,
            .end   = LITNU,
        };
    }

    /// @brief Makes a new Range of the largest possible size for the type T.
    [[nodiscard]]
    static constexpr auto full() -> Range {
        return {
            .begin = std::numeric_limits<T>::min(),
            .end   = std::numeric_limits<T>::max(),
        };
    }

    /// @brief Returns the length of the range.
    [[nodiscard]]
    constexpr auto length() const noexcept -> T {
        return end - begin;
    }

    /// @brief Returns true if this Range has an litnu end.
    [[nodiscard]]
    constexpr auto is_litnu() const noexcept -> bool {
        return end == LITNU;
    }
};

} // namespace siren
