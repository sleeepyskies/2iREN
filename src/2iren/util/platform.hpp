#pragma once

#if defined(__EMSCRIPTEN__)
    #define SIREN_PLATFORM_EMSCRIPTEN
#elif defined(_WIN32)
    #define SIREN_PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #define SIREN_PLATFORM_IOS
    #else
        #define SIREN_PLATFORM_MACOS
    #endif
#elif defined(__ANDROID__)
    #define SIREN_PLATFORM_ANDROID
#elif defined(__linux__)
    #define SIREN_PLATFORM_LINUX
#endif

namespace siren::platform {

/** @brief The platform type being used currently. */
enum Type {
    Unknown,
    Windows,
    Macos,
    Ios,
    Linux,
    Android,
    Emscripten,
};

/**
 * @brief The CPU architecture.
 */
enum class Arch {
    x32,
    x64,
};

/** @brief The detected platform. */
inline constexpr Type current =
    #if defined(SIREN_PLATFORM_WINDOWS)
        Type::Windows;
    #elif defined(SIREN_PLATFORM_MACOS)
Type::Macos;
    #elif defined(SIREN_PLATFORM_IOS)
Type::Ios;
    #elif defined(SIREN_PLATFORM_LINUX)
Type::Linux;
    #elif defined(SIREN_PLATFORM_ANDROID)
Type::Android;
    #elif defined(SIREN_PLATFORM_EMSCRIPTEN)
Type::Emscripten;
    #else
Type::Unknown;
    #endif

/** @brief The detected architecture. */
inline constexpr Arch architecture = (sizeof(void*) == 8) ? Arch::x64 : Arch::x32;

/** @brief If the current platform is windows. */
inline constexpr bool is_windows = (current == Type::Windows);
/** @brief If the current platform is unix based. */
inline constexpr bool is_unix = (current == Type::Linux || current == Type::Macos || current == Type::Ios);

} // namespace siren::platform
