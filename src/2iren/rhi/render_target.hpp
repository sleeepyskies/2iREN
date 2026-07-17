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

struct Attachment {
    enum class Kind : u8 { Color, DepthStencil } kind;
    ImageHandle image;
    BeginOperation begin_operation;
    union Data {
        struct {
            Rgba clear_color;
        } color;

        struct {
            f32 clear_depth;
            i32 clear_stencil;
        } depth_stencil;
    } data;

    static auto create_color(const ImageHandle image, const BeginOperation op, const Rgba color) -> Attachment {
        return Attachment{
            .kind            = Kind::Color,
            .image           = image,
            .begin_operation = op,
            .data            = {.color = {.clear_color = color}},
        };
    }

    static auto create_depth_stencil(
        const ImageHandle image, const BeginOperation op, const f32 clear_depth, const i32 clear_stencil)
        -> Attachment {
        return Attachment{
            .kind            = Kind::DepthStencil,
            .image           = image,
            .begin_operation = op,
            .data            = {.depth_stencil = {.clear_depth = clear_depth, .clear_stencil = clear_stencil}},
        };
    }

    constexpr operator Kind() const { return kind; }
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
