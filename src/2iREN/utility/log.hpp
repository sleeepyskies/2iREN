#pragma once

#include <chrono>
#include <format>
#include <optional>
#include <print>
#include <source_location>
#include <thread>
#include <type_traits>

#include "2iREN/core/assert.hpp"
#include "2iREN/utility/string_utils.hpp"

/// @brief Logging module of 2iREN. Provides a thread safe way to log structured messages to the
/// console.
namespace siren::log {

/**
 * @struct Level
 * @brief Represents the severity level of a log message.
 */
struct Level {
    enum Value { Trace = 0, Debug, Info, Warn, Error, None } value;

    constexpr Level(const Value v) : value(v) {}
    [[nodiscard]] constexpr auto Value() const { return value; }

    /** @brief Returns the string representation of this Level. */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view {
        switch (this->value) {
            case Trace: return "TRACE";
            case Debug: return "DEBUG";
            case Info: return "INFO";
            case Warn: return "WARN";
            case Error: return "ERROR";
            case None: return "NONE";
            default: UNREACHABLE();
        }
    }

    /** @brief Factory method to create a new @ref Level instance from a string. */
    [[nodiscard]] static auto from_string(const std::string_view str) -> std::optional<Level> {
        if (str::equals_ignore_case(str, "trace"))
            return Trace;
        if (str::equals_ignore_case(str, "debug"))
            return Debug;
        if (str::equals_ignore_case(str, "info"))
            return Info;
        if (str::equals_ignore_case(str, "warn"))
            return Warn;
        if (str::equals_ignore_case(str, "error"))
            return Error;
        if (str::equals_ignore_case(str, "none"))
            return None;
        return std::nullopt;
    }

    auto operator<=>(const Level& level) const -> auto = default;
};

namespace impl {
/** @brief The globally configured minimum logging level. Defaults to Info. */
inline Level level{Level::None};
/** @brief Attempts to trim a file path to 2iREN root. */
[[nodiscard]] constexpr auto strip_path(const std::string_view path) -> std::string_view {
    const auto pos = path.find("2iREN/");

    if (pos != std::string_view::npos) {
        return path.substr(pos);
    }

    return path; // fallback to original path if we cant trim somehow
}
} // namespace impl

/**
 * @brief Inits the siren logger with the provided level.
 * @param lvl The desired log level.
 */
inline auto init(const Level lvl) -> void { impl::level = lvl; }

/**
 * @brief Inits the siren logger with the provided level.
 * @param lvl The desired log level as a string.
 */
inline auto init(const std::string_view lvl) -> void {
    const auto level = Level::from_string(lvl);
    ASSERT(level.has_value(), "Passed invalid level to siren::log::init()");
    init(level.value());
}

/**
 * @brief Internal function to format and output log messages.
 * @param lvl The severity of the message.
 * @param color_code The terminal color code to print the message with.
 * @param loc The source code location of the log call.
 * @param fmt The format string.
 * @param args The type-erased format arguments.
 *
 * @todo can we make log output a shortened filepath? it does absolute one atm
 */
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

    const auto usermsg = std::vformat(fmt, args);
    const auto now =
        std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
    const auto threadid = std::this_thread::get_id();
    const std::string locationstring =
        std::format("{}:{}:{}", impl::strip_path(loc.file_name()), loc.line(), loc.column());

    const auto msg = std::format(
        "[{:%F %T}] \033[{}m[{:<5}]\033[0m [thread:{:<15}] [{:<45}] {}",
        now,
        color_code,
        lvl.to_string(),
        threadid,
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
        const T& s, const std::source_location loc = std::source_location::current()
    ) : fmt(s), sl(loc) {}
};

#define LOG_FUNCTION(fn_name, level_val, color_code)                                               \
    template <typename... Args>                                                                    \
    auto fn_name(std::type_identity_t<LogMessage<Args...>> msg, Args&&... args) -> void {          \
        log(level_val, color_code, msg.sl, msg.fmt.get(), std::make_format_args(args...));         \
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

template <HasToString T>
struct std::formatter<T> {
    constexpr auto parse(format_parse_context& ctx) const { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const T& t, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", t.to_string());
    }
};
