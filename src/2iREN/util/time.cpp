#include "time.hpp"

namespace {
using Clock = std::chrono::steady_clock;

bool g_initialized      = false;
siren::u64 g_framecount = 0;
Clock::time_point g_start;
Clock::time_point g_framestart;
Clock::time_point g_lastframe;
} // namespace

namespace siren::time {

auto initialize() -> void {
    if (g_initialized) {
        return;
    }
    g_initialized = true;
    g_framecount  = 0;
    g_start       = Clock::now();
    g_framestart  = g_start;
    g_lastframe   = g_start;
}

auto step() -> void {
    g_framecount++;
    g_lastframe  = g_framestart;
    g_framestart = Clock::now();
}

auto elapsed() -> Duration { return Duration{g_framestart - g_start}; }

auto delta() -> Duration { return Duration{g_framestart - g_lastframe}; }

auto frame_count() -> u32 { return g_framecount; }

} // namespace siren::time
