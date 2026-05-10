#pragma once

#include <iostream>
#include <libenvpp/env.hpp>
#include <libassert/assert.hpp>
#include <optional>
#include <source_location>
#include <type_traits>

#include "string_utils.hpp"

/// @todo: global config instead if env expands more

namespace siren::log {

/**
 * @struct Level
 * @brief Represents the severity level of a log message.
 */
struct Level {
    enum Value {
        Trace = 0,
        Debug,
        Info,
        Warn,
        Error,
        None
    } value;

    constexpr Level(const Value v) : value(v) { }
    constexpr auto Value() const { return value; }

    /** @brief Returns the string representation of this Level. */
    constexpr auto to_string() const -> std::string_view {
        switch (this->value) {
            case Trace: return "Trace";
            case Debug: return "Debug";
            case Info: return "Info";
            case Warn: return "Warn";
            case Error: return "Error";
            case None: return "None";
            default: UNREACHABLE();
        }
    }

    /** @brief Returns the string representation of this Level instance with equal string lengths. */
    constexpr auto to_string_sized() const -> std::string_view {
        switch (this->value) {
            case Trace: return "Trace";
            case Debug: return "Debug";
            case Info: return "Info ";
            case Warn: return "Warn ";
            case Error: return "Error";
            case None: return "None ";
            default: UNREACHABLE();
        }
    }

    /** @brief Factory method to create a new @ref Level instance from a string. */
    static auto from_string(const std::string& str) -> std::optional<Level> {
        if (str::equals_ignore_case(str, "trace")) return Trace;
        if (str::equals_ignore_case(str, "debug")) return Debug;
        if (str::equals_ignore_case(str, "info")) return Info;
        if (str::equals_ignore_case(str, "warn")) return Warn;
        if (str::equals_ignore_case(str, "error")) return Error;
        return std::nullopt;
    }

    auto operator<=>(const Level& level) const -> auto = default;
};

/** @brief The globally configured minimum logging level. Defaults to Info. */
inline Level log_level{ Level::Info };

/**
 * @brief Initializes the logging system by reading the `2IREN_LOG_LEVEL` environment variable.
 * @note Panics if the environment variables cannot be parsed.
 */
inline auto init() -> void {
    env::prefix prefix{ "2IREN" };
    const auto log_lvl = prefix.register_required_variable<std::string>("LOG_LEVEL");
    const auto env     = prefix.parse_and_validate();

    if (!env.ok()) {
        PANIC("Could not read environment variables successfully.");
    }

    log_level = Level::from_string(env.get(log_lvl)).value_or(Level::Info);
}

/**
 * @brief Internal function to format and output log messages.
 * @param level The severity of the message.
 * @param color_code The terminal color code to print the message with.
 * @param loc The source code location of the log call.
 * @param fmt The format string.
 * @param args The type-erased format arguments.
 */
inline void log(
    const Level level,
    const u32 color_code,
    const std::source_location& loc,
    const std::string_view fmt,
    const std::format_args args
) {
    if (level < log_level) { return; }
    std::cout <<
            std::format(
                "\033[{}m[{}]\033[0m  [{}:{}:{}] {}",
                color_code,
                level.to_string_sized(),
                loc.file_name(),
                loc.line(),
                loc.column(),
                std::vformat(fmt, args)
            ) << std::endl;
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

#define LOG_FUNCTION(fn_name, level_val, color_code)                            \
    template <typename... Args>                                                 \
    auto fn_name(                                                               \
        std::type_identity_t<LogMessage<Args...>> msg,                          \
        Args&&... args                                                          \
    ) -> void {                                                                 \
        log(                                                                    \
            level_val,                                                          \
            color_code,                                                         \
            msg.sl,                                                             \
            msg.fmt.get(),                                                      \
            std::make_format_args(args...)                                      \
        );                                                                      \
    }

/**
 * @brief Logs a message at the Level::Trace level.
 * @param msg Message wrapper.
 * @param args Variadic arguments to be formatted.
 */
LOG_FUNCTION(trace, Level::Trace, 90)

/**
 * @brief Logs a message at the Level::Debug level.
 * @param msg Message wrapper.
 * @param args Variadic arguments to be formatted.
 */
LOG_FUNCTION(debug, Level::Debug, 34)

/**
 * @brief Logs a message at the Level::Info level.
 * @param msg Message wrapper.
 * @param args Variadic arguments to be formatted.
 */
LOG_FUNCTION(info, Level::Info, 32)

/**
 * @brief Logs a message at the Level::Warn level.
 * @param msg Message wrapper.
 * @param args Variadic arguments to be formatted.
 */
LOG_FUNCTION(warn, Level::Warn, 33)

/**
 * @brief Logs a message at the Level::Error level.
 * @param msg Message wrapper.
 * @param args Variadic arguments to be formatted.
 */
LOG_FUNCTION(error, Level::Error, 31)

#undef LOG_FUNCTION
} // namespace siren::log
