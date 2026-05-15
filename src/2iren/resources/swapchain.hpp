#pragma once

#include "fwd.hpp"

namespace siren {

/**
 * @struct SwapchainDescriptor
 * @brief Paramters used to create a new @ref Swapchain.
 */
struct SwapchainDescriptor {
    /** @brief An optional label. Not used in OpenGL backend. */
    std::optional<std::string> label;
    /** @brief If the swapchain should be created with vsync enabled. */
    bool vsync;
};

/// @todo: swapchain shouldn't return a framebuffer but an image, but idk how else to make this work for now

/**
 * @class Swapchain
 * @brief A collection of images tied to a specific @ref Window. Used to present images to the screen.
 */
class Swapchain : public RenderResource<Swapchain> {
    using Base = RenderResource<Swapchain>;

public:
    Swapchain(Device* device, SwapchainHandle handle);
    ~Swapchain();
    Swapchain(Swapchain&& other) noexcept;
    Swapchain& operator=(Swapchain&& other) noexcept;

    /** @brief Returns the descriptor of this @ref Swapchain. */
    [[nodiscard]] auto descriptor() const -> const SwapchainDescriptor&;

    /** @brief Returns the next free image managed by this @ref Swapchain to render to. */
    [[nodiscard]] auto current_framebuffer() const -> FramebufferHandle;

    /** @brief Presents the back buffer to the screen. */
    auto present() const -> void;
};
} // namespace siren
