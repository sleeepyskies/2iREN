#pragma once

#include "2iREN/base.hpp"


namespace siren::time {

auto init() -> void;

auto tick() -> void;

auto elapsed_s() -> f32;

auto delta_s() -> f32;

auto elapsed_ms() -> f32;

auto delta_ms() -> f32;

} // namespace siren::time
