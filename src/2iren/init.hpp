#pragma once

#include "util/log.hpp"


namespace siren {

struct InitParams {
    log::Level level = log::Level::Info;
};

auto init(const InitParams& params = { }) -> void;

} // namespace siren
