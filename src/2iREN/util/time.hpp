#pragma once

#include "2iREN/base.hpp"

#include <chrono>

/// @brief A collection of time related utilities.
namespace siren::time {

struct Duration {
    [[nodiscard]]
    constexpr auto nanoseconds() const noexcept -> f64 {
        return value.count();
    }

    [[nodiscard]]
    constexpr auto miliseconds() const noexcept -> f64 {
        return std::chrono::duration<f64, std::milli>{value}.count();
    }

    [[nodiscard]]
    constexpr auto seconds() const noexcept -> f64 {
        return std::chrono::duration<f64>{value}.count();
    }

    std::chrono::nanoseconds value;
};

/// @brief Initializes the time module of 2iREN.
auto initialize() -> void;

/// @brief Updates per frame state.
auto step() -> void;

/// @brief Returns the time elapsed since 2iREN start until the start of the
/// current frame.
[[nodiscard]] auto elapsed() -> Duration;

/// @brief Returns the time elapsed in since the previous frame.
[[nodiscard]] auto delta() -> Duration;

/// @brief Retuns the current frame number.
[[nodiscard]] auto frame_count() -> u32;

} // namespace siren::time
