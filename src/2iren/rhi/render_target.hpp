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

/**
 * @brief A single color attachement in a @ref RenderPass. Descibes what should be
 * drawn to, and how this should be initialized.
 */
struct ColorAttachment {
    /** @brief The handle of the actual @ref Image that will be drawn to. */
    ImageHandle image;
    /** @brief What action to perform before drawing to this target. */
    BeginOperation begin_operation;
    /** @brief The color to clear the target with, if the @ref BeginOperation is Clear. */
    Rgba clear_color;
};

/**
 * @brief A single depth + stencil in a @ref RenderPass. Describes what should be
 * drawn to, and how this should be initialized.
 */
struct DepthStencilAttachment {
    /** @brief The handle of the actual @ref Image that will be drawn to. */
    ImageHandle image;
    /** @brief What action to perform before drawing to this target. */
    BeginOperation begin_operation;
    /** @brief The depth value to clear the target with, if the @ref BeginOperation is Clear. */
    f32 clear_depth;
    /** @brief The stencil value to clear the target with, if the @ref BeginOperation is Clear. */
    i32 clear_stencil;
};

/**
 * @struct RenderTarget
 * @brief Describes a target for a render pass.
 */
struct RenderTarget {
    /** @brief All color attachments of this target. May have size 0. */
    std::vector<ColorAttachment> colors;
    /** @brief Optional depth + stencil attachment. */
    std::optional<DepthStencilAttachment> depth_stencil = std::nullopt;
};

} // namespace siren
