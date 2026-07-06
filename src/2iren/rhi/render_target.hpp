#pragma once
#include "resources/fwd.hpp"

namespace siren {

/**
 * @struct RenderTarget
 * @brief Describes a target for a render pass.
 * @note Atm, this is simply just an image wrapper, but eventually more images and fields may be added.
 */
struct RenderTarget {
    explicit RenderTarget(const ImageHandle color) : color(color) {}
    ImageHandle color;
};

} // namespace siren
