#pragma once

#include <string>

#include "2iREN/base.hpp"


namespace siren::str {

/**
 * @brief Converts an input string into lower case.
 * @param str The string to convert into lower case.
 * @return A new lower case version of the input string.
 */
constexpr auto tolower(const std::string& str) -> std::string {
    auto lwr = str;
    for (auto& c : lwr) { c = static_cast<char>(std::tolower(c)); }
    return lwr;
}

/**
 * @brief Converts an input string into upper case.
 * @param str The string to convert into upper case.
 * @return A new upper case version of the input string.
 */
constexpr auto toupper(const std::string& str) -> std::string {
    auto lwr = str;
    for (auto& c : lwr) { c = static_cast<char>(std::toupper(c)); }
    return lwr;
}

/**
 * @brief Checks two provided strings for case-insensitive equality.
 * @param left The first string to compare.
 * @param right The second string to compare.
 * @return True if the strings are equal ignoring case, false otherwise.
 */
constexpr auto equals_ignore_case(const std::string_view& left, const std::string_view& right) -> bool {
    if (left.size() != right.size()) { return false; }
    for (usize i = 0; i < left.size(); i++) {
        if (std::tolower(left[i]) != std::tolower(right[i])) {
            return false;
        }
    }
    return true;
}

} // namespace siren::str
