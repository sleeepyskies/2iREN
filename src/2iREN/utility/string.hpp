#pragma once

#include <ranges>
#include <string>

#include "2iREN/base.hpp"

namespace siren::string {

/// @brief Converts a string to lower case.
[[nodiscard]]
constexpr inline auto tolower(const std::string_view str) noexcept -> std::string {
    return str
        | std::views::transform([](auto c) { return std::tolower(c); })
        | std::ranges::to<std::string>();
}

/// @brief Converts a string to upper case.
[[nodiscard]]
constexpr inline auto toupper(const std::string_view str) noexcept -> std::string {
    return str
        | std::views::transform([](auto c) { return std::toupper(c); })
        | std::ranges::to<std::string>();
}

/// @brief Checks equality of two strigs ignoring case.
[[nodiscard]]
constexpr inline auto equals_ignore_case(
    const std::string_view left,
    const std::string_view right
) noexcept -> bool {
    if (left.size() != right.size()) {
        return false;
    }

    for (usize i = 0; i < left.size(); i++) {
        if (std::tolower(left[i]) != std::tolower(right[i])) {
            return false;
        }
    }

    return true;
}

} // namespace siren::string
