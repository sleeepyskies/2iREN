#pragma once
#include "resources/fwd.hpp"
#include "resources/image.hpp"

namespace siren {

/**
 * @struct RenderTarget
 * @brief Describes a target for a render pass.
 * @note Atm, this is simply just an image wrapper, but eventually more images and fields may be added.
 * @todo Not great, in cases where the render target should be owning this kinda fails as the caller
 * has to ensure lifetime.
 */
struct RenderTarget {
    explicit RenderTarget(const ImageHandle color) : color(color) {}
    ImageHandle color;
};

} // namespace siren
