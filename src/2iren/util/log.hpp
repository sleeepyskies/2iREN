#pragma once

#include <iostream>
#include <optional>
#include <source_location>
#include <type_traits>
#include <format>
#include <libassert/assert.hpp>

#include "2iren/sync/mutex.hpp"
#include "string_utils.hpp"

/// @todo:
///     log functions show error msg in my IDE but still compile and run... fix that prolly
///     this seems to be a clang bug though??? annoying...

namespace siren::log {

namespace detail {
/** @brief Global mutex used by the logger. */
inline Mutex<void> log_mutex{ };
}


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
            case Info: return "Info ";
            case Warn: return "Warn ";
            case Error: return "Error";
            case None: return "None ";
            default: UNREACHABLE();
        }
    }

    /** @brief Factory method to create a new @ref Level instance from a string. */
    static auto from_string(const std::string_view str) -> std::optional<Level> {
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
inline Level level{ Level::None };

/**
 * @brief Inits the siren logger with the provided level.
 * @param lvl The desired log level.
 */
inline auto init(const Level lvl) -> void { level = lvl; }

/**
 * @brief Inits the siren logger with the provided level.
 * @param lvl The desired log level as a string.
 */
inline auto init(const std::string_view lvl) -> void {
    const auto level_ = Level::from_string(lvl);
    ASSERT(level_.has_value(), "Passed invalid level to siren::log::init()");
    init(level_.value());
}

/**
 * @brief Internal function to format and output log messages.
 * @param lvl The severity of the message.
 * @param color_code The terminal color code to print the message with.
 * @param loc The source code location of the log call.
 * @param fmt The format string.
 * @param args The type-erased format arguments.
 */
inline void log(
    const Level lvl,
    const u32 color_code,
    const std::source_location& loc,
    const std::string_view fmt,
    const std::format_args args
) {
    if (lvl < level) { return; }

    const auto msg = std::format(
        "\033[{}m[{}]\033[0m  [{}:{}:{}] {}",
        color_code,
        lvl.to_string(),
        loc.file_name(),
        loc.line(),
        loc.column(),
        std::vformat(fmt, args)
    );

    if constexpr (single_threaded) {
        std::cout << msg << std::endl;
    } else {
        detail::log_mutex.run([msg = std::move(msg)]{ std::cout << msg << std::endl; });
    }
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

template <typename T>
concept HasToString = requires(const T& t) {
    { t.to_string() } -> std::convertible_to<std::string_view>;
};

template<HasToString T>
struct std::formatter<T> {
    constexpr auto parse(format_parse_context& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const T& t, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", t.to_string());
    }
};
