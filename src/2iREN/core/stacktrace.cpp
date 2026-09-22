#include "stacktrace.hpp"

#include <cstdlib>
#include <format>
#include <version>

#ifdef __cpp_lib_stacktrace
#include <stacktrace>
#elif defined(SIREN_LINUX) || defined(SIREN_MACOS)
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#endif

namespace siren {

#if !defined(__cpp_lib_stacktrace) && (defined(SIREN_LINUX) || defined(SIREN_MACOS))
namespace {
auto demangle(const char* symbol) -> std::string {
    i32 status = 0;

    const auto name   = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
    const auto result = std::string{status == 0 ? name : symbol};

    std::free(name);
    return result;
}
} // namespace
#endif

auto Stacktrace::make(const u32 skip) -> Stacktrace {
    Stacktrace trace;
    if (skip >= 128) {
        return trace;
    }

#ifdef __cpp_lib_stacktrace
    const auto st = std::stacktrace::current(skip);
    trace.stack   = std::format("{}", st);
#elif defined(SIREN_LINUX) || defined(SIREN_MACOS)
    void* addrs[128];
    const i32 written = backtrace(addrs, 128);
    char** strs       = backtrace_symbols(addrs, written);

    if (!strs) {
        return trace;
    }

    // skip last entry as is not in binary
    for (i32 i = static_cast<i32>(skip); i < written - 1; i++) {
        auto line = std::string{strs[i]};
        auto info = Dl_info{};

        if (dladdr(addrs[i], &info) and info.dli_sname) {
            const auto symbol = std::string_view{info.dli_sname};
            const auto pos    = line.find(symbol);

            if (pos != std::string::npos) {
                line.replace(pos, symbol.size(), demangle(info.dli_sname));
            }
        }

        trace.stack += std::format("{}\n", line);
    }

    std::free(strs);
#endif

    return trace;
}

} // namespace siren
