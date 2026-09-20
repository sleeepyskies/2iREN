#pragma once

#include <optional>

#include "2iREN/graphics/device.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/math/extent.hpp"

namespace siren {

class Window;

/// @brief Represents the number of frames there can be in flight at once.
/// May be any value in [1, 3]. Set to 1 for normal rendering, 2 for double
/// buffering and 3 for triple buffering.
using FramesInFlight = BoundedU32<1, 3>;

/// @brief Parameters used to update a @ref Swapchain.
struct SwapchainDescriptor {
    /// @brief The size of the swapchains underlying framebuffer in pixels.
    std::optional<Extent2u> extent          = std::nullopt;
    /// @brief If the swapchain should be created with vsync enabled.
    std::optional<bool> vsync               = std::nullopt;
    /// @brief The format of the pixels of the swapchains image.
    std::optional<ImageFormat> image_format = std::nullopt;
    // std::optional<FramesInFlight> frames_in_flight = std::nullopt;
};

/// @brief Parameters of the swapchain.
struct SwapchainInfo {
    /// @brief The size of the swapchains underlying framebuffer in pixels.
    Extent2u extent;
    /// @brief If the swapchain has vsync enabled.
    bool vsync;
    /// @brief The format of the pixels of the swapchains image.
    ImageFormat image_format;
    // std::optional<FramesInFlight> frames_in_flight = std::nullopt;
};

/// @brief A collection of images tied to a specific @ref Window. Used to present
/// images to the screen.
class Swapchain : public RenderResource<Swapchain> {
    using Base = RenderResource<Swapchain>;

public:
    Swapchain(Device* device, SwapchainHandle handle);
    ~Swapchain();
    Swapchain(Swapchain&& other) noexcept;
    Swapchain& operator=(Swapchain&& other) noexcept;

    /// @brief Returns the descriptor of this @ref Swapchain.
    [[nodiscard]]
    auto info() const -> const SwapchainInfo&;

    /// @brief Resizes the swapchain.
    auto update(const SwapchainDescriptor& new_values) -> void;

    /// @brief Returns the next free image managed by this @ref Swapchain to
    /// render to.
    [[nodiscard]]
    auto next_image() const -> ImageHandle;

    /// @brief Presents the back buffer to the screen.
    auto present() const -> void;

    /// @brief Presents the back buffer to the screen after the commands have finished executing.
    auto present(std::unique_ptr<CommandBuffer>&& command_buffer) const -> void;
};
} // namespace siren
