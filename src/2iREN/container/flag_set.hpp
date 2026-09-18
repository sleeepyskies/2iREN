#pragma once

#include <bitset>
#include <type_traits>

#include "2iREN/utility/concepts.hpp"

namespace siren {

template <IsSizedEnum F>
class FlagSet {

public:
    static constexpr auto FlagCount = std::to_underlying(F::Max);
    using Flag                      = std::remove_cvref_t<F>;
    using UnderlyingType            = std::underlying_type_t<Flag>;

    [[nodiscard]]
    static constexpr auto empty() noexcept -> FlagSet {
        return FlagSet{};
    }
    [[nodiscard]]
    static constexpr auto from(IsSameType<F> auto... flags) noexcept -> FlagSet {
        auto set = FlagSet{};
        (set.set(flags), ...);
        return set;
    }

    constexpr auto set(const Flag flag) noexcept -> void;

    constexpr auto unset(const Flag flag) noexcept -> void;

    constexpr auto operator|=(const Flag flag) -> void {
        set(flag);
    }

    [[nodiscard]]
    constexpr auto test(const Flag flag) const noexcept -> bool;

    [[nodiscard]]
    constexpr auto all(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... && test(flags));
    }

    [[nodiscard]]
    constexpr auto any(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... || test(flags));
    }

    [[nodiscard]]
    constexpr auto none(IsSameType<F> auto... flags) const noexcept -> bool {
        return (... && !test(flags));
    }

private:
    FlagSet()                       = default;
    std::bitset<FlagCount> m_values = {};
};

} // namespace siren
