#pragma once

#include <cstdlib>
#include <format>
#include <iostream>
#include <print>
#include <source_location>
#include <string_view>
#include <thread>
#include <utility>
#include <stacktrace>

namespace siren::impl {

/** @brief Attempts to trim a file path to 2iREN root. */
[[nodiscard]] constexpr auto strip_path(const std::string_view path) -> std::string_view {
    const auto pos = path.find("2iREN/");

    if (pos != std::string_view::npos) {
        return path.substr(pos);
    }

    return path; // fallback to original path if we cant trim somehow
}

[[noreturn]] inline auto report_and_terminate(
    const std::source_location& location,
    const std::string_view expression,
    const std::string_view message
) -> void {
    const std::string locationstring =
        std::format("{}:{}:{}", strip_path(location.file_name()), location.line(), location.column());

    std::println(
        std::cerr,
        "thread {} panicked during {} at {}",
        std::this_thread::get_id(),
        location.function_name(),
        locationstring
    );

    if (!expression.empty()) {
        std::println(std::cerr, "assertion: {}", expression);
    }

    if (!message.empty()) {
        std::println(std::cerr, "message: {}", message);
    }

    if (const auto trace = std::stacktrace::current(); !trace.empty()) {
        std::println(std::cerr, "stack trace:\n{}", trace);
    }

    std::abort();
}

[[noreturn]] inline auto do_panic(const std::source_location& location) -> void {
    report_and_terminate(location, "", "");
}

[[noreturn]] inline auto do_panic(
    const std::source_location& location, const std::string_view message
) -> void {
    report_and_terminate(location, "", message);
}

template <typename... Args>
[[noreturn]] inline auto do_panic(
    const std::source_location& location, const std::format_string<Args...> format, Args&&... args
) -> void {
    do_panic(location, std::format(format, std::forward<Args>(args)...));
}

[[noreturn]] inline auto do_unreachable(const std::source_location& location) -> void {
    do_panic(location, "unreachable code reached");
}

[[noreturn]] inline auto do_unreachable(
    const std::source_location& location, const std::string_view message
) -> void {
    do_panic(location, message);
}

template <typename... Args>
[[noreturn]] inline auto do_unreachable(
    const std::source_location& location, const std::format_string<Args...> format, Args&&... args
) -> void {
    do_panic(location, format, std::forward<Args>(args)...);
}

[[noreturn]] inline auto do_assertion_failed(
    const std::source_location& location, const std::string_view expression
) -> void {
    report_and_terminate(location, expression, "");
}

[[noreturn]] inline auto do_assertion_failed(
    const std::source_location& location,
    const std::string_view expression,
    const std::string_view message
) -> void {
    report_and_terminate(location, expression, message);
}

template <typename... Args>
[[noreturn]] inline auto do_assertion_failed(
    const std::source_location& location,
    const std::string_view expression,
    const std::format_string<Args...> format,
    Args&&... args
) -> void {
    do_assertion_failed(location, expression, std::format(format, std::forward<Args>(args)...));
}

} // namespace siren::impl

/// @brief Crashes the program with an optional formatted message and stack trace.
#define PANIC(...) siren::impl::do_panic(std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)

/// @brief Crashes the program because control reached a logically impossible path.
#define UNREACHABLE(...)                                                                           \
    siren::impl::do_unreachable(std::source_location::current() __VA_OPT__(, ) __VA_ARGS__)

/// @brief Checks a condition and crashes with an optional formatted message when it is false.
#define ASSERT(condition, ...)                                                                     \
    do {                                                                                           \
        if (!(condition)) [[unlikely]] {                                                           \
            siren::impl::do_assertion_failed(                                                      \
                std::source_location::current(), #condition __VA_OPT__(, ) __VA_ARGS__             \
            );                                                                                     \
        }                                                                                          \
    } while (false)
