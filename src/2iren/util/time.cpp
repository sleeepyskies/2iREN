#include "time.hpp"

#include <chrono>


namespace siren::time {

// default duration of steady clock is nanoseconds

using Clock     = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

static TimePoint init_time;
static TimePoint last_frame;
static TimePoint now;

auto init() -> void {
    now        = std::chrono::steady_clock::now();
    init_time  = now;
    last_frame = now;
}

auto tick() -> void {
    last_frame = now;
    now        = std::chrono::steady_clock::now();
}

auto elapsed_s() -> f32 {
    return std::chrono::duration<f32>(now - init_time).count();
}

auto delta_s() -> f32 {
    return std::chrono::duration<f32>(now - last_frame).count();
}

auto elapsed_ms() -> f32 {
    return elapsed_s() * 1000;
}

auto delta_ms() -> f32 {
    return delta_s() * 1000;
}

} // namespace siren::time
