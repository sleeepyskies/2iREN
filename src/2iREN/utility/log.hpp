#pragma once

// TODO: what warning flag to disable this one?
// do not remove this include!!
#include "2iREN/core/format.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <optional>
#include <print>
#include <ranges>
#include <source_location>
#include <type_traits>

#include "2iREN/core/assert.hpp"
#include "2iREN/utility/string.hpp"

/// @brief Logging module of 2iREN. Provides a thread safe way to
/// log structured messages to the console.
namespace siren::log {

/// @brief Represents the severity level of a log message.
struct Level {
    enum Value { Trace = 0, Debug, Info, Warn, Error, None } value;

    constexpr Level(const Value v) : value(v) { }
    [[nodiscard]] constexpr auto Value() const {
        return value;
    }

    [[nodiscard]]
    constexpr auto to_string() const -> std::string_view {
        switch (value) {
            case Trace: return "TRACE";
            case Debug: return "DEBUG";
            case Info: return "INFO";
            case Warn: return "WARN";
            case Error: return "ERROR";
            case None: return "NONE";
            default: UNREACHABLE();
        }
    }

    [[nodiscard]]
    static auto from_string(const std::string_view str) -> std::optional<Level> {
        if (string::equals_ignore_case(str, "trace"))
            return Trace;
        if (string::equals_ignore_case(str, "debug"))
            return Debug;
        if (string::equals_ignore_case(str, "info"))
            return Info;
        if (string::equals_ignore_case(str, "warn"))
            return Warn;
        if (string::equals_ignore_case(str, "error"))
            return Error;
        if (string::equals_ignore_case(str, "none"))
            return None;
        return std::nullopt;
    }

    auto operator<=>(const Level& level) const -> auto = default;
};

namespace impl {

inline Level level{Level::None};

[[nodiscard]]
inline auto prettify_path(std::string_view path) -> std::string {
    return std::filesystem::path(path)
        | std::views::reverse
        | std::views::take(2)
        | std::views::transform([](const std::filesystem::path& part) { return part.string(); })
        | std::views::reverse
        | std::views::join_with(std::string_view{"/"})
        | std::ranges::to<std::string>();
}

} // namespace impl

/// @brief Inits the siren logger with the provided level.
/// @param lvl The desired log level.
inline auto initialize(const Level lvl) -> void {
    impl::level = lvl;
}

/// @brief Internal function to format and output log messages.
/// @param lvl The severity of the message.
/// @param color_code The terminal color code to print the message with.
/// @param loc The source code location of the log call.
/// @param fmt The format string.
/// @param args The type-erased format arguments.
inline void log(
    const Level lvl,
    const u32 color_code,
    const std::source_location& loc,
    const std::string_view fmt,
    const std::format_args args
) {
    if (lvl < impl::level) {
        return;
    }

    const auto usermsg               = std::vformat(fmt, args);
    const auto now                   = std::chrono::system_clock::now();
    const std::string locationstring = std::format(
        "{}",
        impl::prettify_path(loc.file_name()),
        loc.function_name()
    );

    const auto msg = std::format(
        "\033[38;5;242m{:%FT:%T}\033[0m \033[{}m{:>5}\033[0m \033[38;5;242m{}\033[0m {}",
        now,
        color_code,
        lvl.to_string(),
        locationstring,
        usermsg
    );

    std::println("{}", msg);
}

template <typename... Args>
struct LogMessage {
    std::format_string<Args...> fmt;
    std::source_location sl;

    template <typename T>
    consteval LogMessage(
        const T& s,
        const std::source_location loc = std::source_location::current()
    ) : fmt(s), sl(loc) { }
};

#define LOG_FUNCTION(fn_name, level_val, color_code)                                               \
    template <typename... Args>                                                                    \
    auto fn_name(std::type_identity_t<LogMessage<Args...>> msg, Args&&... args) -> void {          \
        log(level_val, color_code, msg.sl, msg.fmt.get(), std::make_format_args(args...));         \
    }

/// @brief Logs a message at the Level::Trace level.
/// @param msg Message wrapper.
/// @param args Variadic arguments to be formatted.
LOG_FUNCTION(trace, Level::Trace, 35)

/// @brief Logs a message at the Level::Debug level.
/// @param msg Message wrapper.
/// @param args Variadic arguments to be formatted.
LOG_FUNCTION(debug, Level::Debug, 34)

/// @brief Logs a message at the Level::Info level.
/// @param msg Message wrapper.
/// @param args Variadic arguments to be formatted.
LOG_FUNCTION(info, Level::Info, 32)

/// @brief Logs a message at the Level::Warn level.
/// @param msg Message wrapper.
/// @param args Variadic arguments to be formatted.
LOG_FUNCTION(warn, Level::Warn, 33)

/// @brief Logs a message at the Level::Error level.
/// @param msg Message wrapper.
/// @param args Variadic arguments to be formatted.
LOG_FUNCTION(error, Level::Error, 31)

#undef LOG_FUNCTION
} // namespace siren::log
