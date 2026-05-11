#pragma once

#include <string>
#include <optional>

#include "2iren/base.hpp"
#include "image.hpp"
#include "fwd.hpp"


namespace siren {

/**
 * @brief Describes a @ref Framebuffer. Used for object creation via @ref Device.
 */
struct FramebufferDescriptor {
    /** @brief The optional label of the @ref Framebuffer. Mainly used for debugging. */
    std::optional<std::string> label;
    /** @brief The width of the desired @ref Framebuffer in pixels. */
    u32 width;
    /** @brief The height of the desired @ref Framebuffer in pixels. */
    u32 height;
    /** @brief The number of color attachments. */
    u32 num_colors;
    /** @brief Whether there's number of color attachments. */
    bool has_depth_stencil;
};

/**
 * @class Framebuffer
 * @brief Represents a GPU framebuffer object along with its associated attachments.
 * Is essentially a collection of @ref Image's for color, depth and stencil attachments.
 * @todo: store images in the backend instead?
 */
class Framebuffer final : public RenderResource<Framebuffer> {
public:
    using Base = RenderResource<Framebuffer>;
    Framebuffer(
        Device* device,
        FramebufferHandle handle,
        std::vector<Image>&& colors,
        std::optional<Image>&& depth_stencil
    );
    ~Framebuffer();

    Framebuffer(Framebuffer&& other) noexcept;
    Framebuffer& operator=(Framebuffer&& other) noexcept;

    /** @brief Returns the @ref FramebufferDescriptor used to create this Framebuffer. */
    [[nodiscard]] auto descriptor() const noexcept -> const FramebufferDescriptor&;

    /** @brief Returns the color attachment of this Framebuffer. */
    [[nodiscard]] auto color_attachment(usize index) const noexcept -> const Image*;
    /** @brief Returns the depth_stencil attachment of this Framebuffer. */
    [[nodiscard]] auto depth_stencil_attachment() const noexcept -> const Image*;

private:
    /** @brief List of all color attachments. */
    std::vector<Image> m_colors;
    /** @brief The optional depth_stencil attachment. */
    std::optional<Image> m_depth_stencil;
};
} // namespace siren
