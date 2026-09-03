#pragma once

#include <format>

namespace siren {

/// @brief Ensures a type can be represented as a string.
/// This may be either a free or a member function.
template <typename T>
concept HasToString = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string_view>;
} || requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string_view>;
};

} // namespace siren

template <siren::HasToString T>
struct std::formatter<T> {
    constexpr auto parse(format_parse_context& ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const T& t, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", t.to_string());
    }
};
