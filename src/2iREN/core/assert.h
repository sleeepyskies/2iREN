#include <iostream>
#include <source_location>
#include <thread>
#include "2iREN/util/log.hpp"

namespace siren {

namespace impl {

// thread name panicked during function at file.h:line:column
inline auto do_panic(const std::string_view format_string, const std::source_location& location) {
    log::error();
    std::println(
        std::cerr,
        "thread {} panicked during {} at {}:{}:{}",
        std::this_thread::get_id(),
        location.file_name(),
        location.function_name(),
        location.file_name(),
        location.line(),
        location.column()
    );
    std::terminate();
}

} // namespace impl

/// @brief Crashes the program with the provided message and outputs the
/// stacktrace.
#define panic(msg, args) do_panic()

/// @brief Checks the given condition. If true, the program continues, if false,
/// the program will panic.
#define assert(cond, msg, args)                                                                    \
    do {                                                                                           \
        if (cond) {                                                                                \
            panic(msg, args);                                                                      \
        }                                                                                          \
    } while (false)

} // namespace siren
