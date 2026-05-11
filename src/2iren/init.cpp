#include "init.hpp"


namespace siren {

static bool initialized = false;

auto init(const InitParams& params) -> void {
    if (initialized) {
        return;
    }
    initialized = true;

    log::init(params.level);
}

} // namespace siren
