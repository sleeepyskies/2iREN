#pragma once

#include <optional>
#include <vector>

#include "2iren/util/color.hpp"
#include "resources/fwd.hpp"

namespace siren {

/**
 * @brief Indicates the operation to perform on a new pass.
 */
enum class BeginOperation : u8 {
    /** @brief Clears the target a single color. */
    Clear,
    /** @brief Keeps the content of the attachment. */
    Preserve,
    /** @brief Doesnt even load the previous data. */
    Fuckit,
};

struct ColorAttachment {
    ImageHandle image;
    BeginOperation begin_operation;
    Rgba clear_color;
};

struct DepthStencilAttachment {
    ImageHandle image;
    BeginOperation begin_operation;
    f32 clear_depth;
    i32 clear_stencil;
};

/**
 * @struct RenderTarget
 * @brief Describes a target for a render pass.
 * @todo Not great, in cases where the render target should be owning this kinda fails as the caller
 * has to ensure lifetime.
 */
struct RenderTarget {
    std::vector<ColorAttachment> colors;
    std::optional<DepthStencilAttachment> depth_stencil = std::nullopt;
};

} // namespace siren
