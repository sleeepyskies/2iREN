#pragma once

#include "resources/fwd.hpp"

namespace siren {

/**
 * @struct RenderTarget
 * @brief Describes a target for a render pass.
 * @todo Not great, in cases where the render target should be owning this kinda fails as the caller
 * has to ensure lifetime.
 */
struct RenderTarget {
    std::vector<ImageHandle> colors;
    std::optional<ImageHandle> depth_stencil;
};

} // namespace siren
