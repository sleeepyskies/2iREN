#pragma once

#include "2iREN/rhi/device.hpp"
#include "fwd.hpp"

namespace siren {

class Window;

/**
 * @struct SwapchainDescriptor
 * @brief Parameters used to create a new @ref Swapchain.
 */
struct SwapchainDescriptor {
    /** @brief An optional label. Not used in OpenGL backend. */
    std::optional<std::string> label;
    /** @brief If the swapchain should be created with vsync enabled. */
    bool vsync;
    /** @brief The size dimensions of the desired @ref Swapchain. */
    glm::uvec2 extent;
    /** @brief The @ref Window to associate the @ref Swapchain with. */
    Window* window;
};

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
    [[nodiscard]] auto next_image() const -> ImageHandle;

    /** @brief Presents the back buffer to the screen. */
    auto present() const -> void;

    /**
     * @brief Presents the back buffer to the screen and also executes the overlay function.
     * @todo this api isn't great, but works for now and keeps hard dependency in 2iREN core for ImGui out.
     */
    auto present_overlay(OverlayFunction&& overlay) const -> void;
};
} // namespace siren
