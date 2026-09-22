#pragma once

#include <string_view>

#include "2iREN/core/base.hpp"
#include "2iREN/core/format.hpp" // do not remove

namespace siren {

/// @brief Object representing the current threads call stack.
/// @note We use a custom wrapper as <stacktrace> is not supported yet on
/// clang and apple-clang.
///
/// This Stacktrace will internally try to use first <stacktrace>,
/// then <execinfo.h> on unix systems, otherwise no stacktrace is emitted.
struct Stacktrace {
    [[nodiscard]]
    static auto make(u32 skip) -> Stacktrace;

    /// @brief Returns the configured stacktrace string.
    [[nodiscard]]
    constexpr auto to_string() const noexcept -> std::string_view {
        return stack;
    }

    /// @brief Checks if a stacktrace is held.
    [[nodiscard]]
    constexpr auto empty() const noexcept -> bool {
        return stack.empty();
    }

    std::string stack;
};

} // namespace siren
