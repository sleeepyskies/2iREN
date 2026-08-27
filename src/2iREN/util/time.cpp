#include "time.hpp"

#include <chrono>

#include "log.hpp"

namespace siren::time {

// default duration of steady clock is nanoseconds

using Clock     = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;

static TimePoint init_time;
static TimePoint last_frame;
static TimePoint now;

auto init() -> void {
    now        = Clock::now();
    init_time  = now;
    last_frame = now;
}

auto tick() -> void {
    last_frame = now;
    now        = Clock::now();
}

auto elapsed_s() -> f32 { return std::chrono::duration<f32>(now - init_time).count(); }
auto delta_s() -> f32 { return std::chrono::duration<f32>(now - last_frame).count(); }
auto elapsed_ms() -> f32 { return elapsed_s() * 1000; }
auto delta_ms() -> f32 { return delta_s() * 1000; }

} // namespace siren::time
