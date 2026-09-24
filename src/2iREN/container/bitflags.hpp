#pragma once

#include <bitset>
#include <format>
#include <type_traits>
#include <utility>

#include "2iREN/utility/concepts.hpp"

namespace siren {

template <typename T>
inline constexpr auto HasBitFlags = false;

/// @brief BitFlags represents a dynamic combination of flags provided by an enum.
template <IsSizedEnum F>
class BitFlags {
public:
    static constexpr auto FlagCount = std::to_underlying(F::Max);
    using FlagType                  = std::remove_cvref_t<F>;
    using UnderlyingType            = std::underlying_type_t<FlagType>;

    BitFlags() = default;

    /// @brief Constructs a new BitFlags from a single flag.
    BitFlags(const FlagType flag) : m_values(make(flag).m_values) { }

    /// @brief Creates a new BitFlags with all of the provided flags set.
    /// Can also be used to create an empty set if provided no arguments.
    [[nodiscard]]
    static constexpr auto make(IsSameType<F> auto... flags) noexcept -> BitFlags {
        auto set = BitFlags{};
        (set.set(flags), ...);
        return set;
    }

    /// @brief Sets the provided flag in the flag set.
    constexpr auto set(const FlagType flag) noexcept -> void {
        m_values.set(std::to_underlying(flag));
    }

    /// @brief Unsets the provided flag in the flag set.
    constexpr auto unset(const FlagType flag) noexcept -> void {
        m_values.reset(std::to_underlying(flag));
    }

    /// @brief Unsets all flags.
    constexpr auto reset() noexcept -> void {
        m_values.reset();
    }

    /// @brief Sets the provided flag in the flag set.
    constexpr auto operator|=(const FlagType flag) -> void {
        set(flag);
    }

    /// @brief Checks if the provided flag is set.
    [[nodiscard]]
    constexpr auto test(const FlagType flag) const noexcept -> bool {
        return m_values.test(std::to_underlying(flag));
    }

    /// @brief Checks if all of the provided flags are set.
    [[nodiscard]]
    constexpr auto all(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... and test(flags));
    }

    /// @brief Checks if all of this sets flags are set.
    [[nodiscard]]
    constexpr auto all() const noexcept -> bool {
        return m_values.all();
    }

    /// @brief Checks if any of the provided flags are set.
    [[nodiscard]]
    constexpr auto any(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... or test(flags));
    }

    /// @brief Checks if any of this sets flags are set.
    [[nodiscard]]
    constexpr auto any() const noexcept -> bool {
        return m_values.any();
    }

    /// @brief Checks if none of the provided flags are set.
    [[nodiscard]]
    constexpr auto none(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... and !test(flags));
    }

    /// @brief Checks if none of this sets flags are set.
    [[nodiscard]]
    constexpr auto none() const noexcept -> bool {
        return m_values.none();
    }

    /// @brief Returns a string representation of this BitFlags.
    [[nodiscard]]
    constexpr auto to_string() const noexcept -> std::string {
        return std::format("BitFlags<{}>({})", typename_of<F>(), m_values.to_string());
    }

private:
    std::bitset<FlagCount> m_values = {};
};

template <IsSizedEnum F>
inline auto operator|(const F left, const F right) -> BitFlags<F> {
    return BitFlags<F>::make(left, right);
}

template <IsSizedEnum F>
constexpr auto operator|(BitFlags<F> left, const F right) noexcept -> BitFlags<F> {
    left.set(right);
    return left;
}

} // namespace siren
