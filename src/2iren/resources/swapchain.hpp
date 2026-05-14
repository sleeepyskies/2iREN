#pragma once

#include "fwd.hpp"

namespace siren {

struct SwapchainDescriptor {
    std::optional<std::string> label;
};

/// @todo: swapchain shouldn't return a framebuffer but an image, but idk how else to make this work for now

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

    auto present() const -> void;
};
} // namespace siren
