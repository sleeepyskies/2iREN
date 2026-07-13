#pragma once

#include "resources/fwd.hpp"

namespace siren {

/**
 * @brief Indicates the operation to perform on a new pass.
 */
enum class BeginOperation : u8 {
    /** @brief Clears the target a single color. */
    Clear,
    /** @brief Does nothing. */
    Fuckit,
};

/**
 * @brief Describes an attachment of a @ref RenderTarget as well as how to handle it.
 */
struct Attachment {
    /** @brief The value to clear the image with. */
    glm::vec4 clear_value          = glm::vec4{0.0};
    BeginOperation begin_operation = BeginOperation::Clear;
    ImageHandle image;
};

/**
 * @struct RenderTarget
 * @brief Describes a target for a render pass.
 * @todo Not great, in cases where the render target should be owning this kinda fails as the caller
 * has to ensure lifetime.
 */
struct RenderTarget {
    std::vector<Attachment> colors;
    std::optional<Attachment> depth_stencil;
};

} // namespace siren
