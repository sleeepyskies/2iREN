#pragma once

#include <bitset>
#include <type_traits>
#include <utility>

#include "2iREN/utility/concepts.hpp"

namespace siren {

/// @brief FlagSet represents a dynamic combination of flags provided
/// by an enum.
template <IsSizedEnum F>
class FlagSet {
public:
    static constexpr auto FlagCount = std::to_underlying(F::Max);
    using Flag                      = std::remove_cvref_t<F>;
    using UnderlyingType            = std::underlying_type_t<Flag>;

    FlagSet() = default;

    /// @brief Creates a new FlagSet with no flags set.
    [[nodiscard]]
    static constexpr auto empty() noexcept -> FlagSet {
        return FlagSet{};
    }

    /// @brief Creates a new FlagSet with all of the provided flags set.
    [[nodiscard]]
    static constexpr auto from(IsSameType<F> auto... flags) noexcept -> FlagSet {
        auto set = FlagSet{};
        (set.set(flags), ...);
        return set;
    }

    /// @brief Sets the provided flag in the flag set.
    constexpr auto set(const Flag flag) noexcept -> void {
        m_values.set(std::to_underlying(flag));
    }

    /// @brief Unsets the provided flag in the flag set.
    constexpr auto unset(const Flag flag) noexcept -> void {
        m_values.reset(std::to_underlying(flag));
    }

    /// @brief Sets the provided flag in the flag set.
    constexpr auto operator|=(const Flag flag) -> void {
        set(flag);
    }

    /// @brief Checks if the provided flag is set.
    [[nodiscard]]
    constexpr auto test(const Flag flag) const noexcept -> bool {
        return m_values.test(std::to_underlying(flag));
    }

    /// @brief Checks if all of the provided flags are set.
    [[nodiscard]]
    constexpr auto all(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... && test(flags));
    }

    /// @brief Checks if any of the provided flags are set.
    [[nodiscard]]
    constexpr auto any(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... || test(flags));
    }

    /// @brief Checks if none of the provided flags are set.
    [[nodiscard]]
    constexpr auto none(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... && !test(flags));
    }

private:
    std::bitset<FlagCount> m_values = {};
};

} // namespace siren
