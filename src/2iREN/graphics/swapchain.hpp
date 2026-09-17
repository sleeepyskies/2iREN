#pragma once

#include "2iREN/graphics/device.hpp"
#include "fwd.hpp"

namespace siren {

class Window;

/// @brief Represents the number of frames there can be in flight at once.
/// May be any value in [1, 3]. Set to 1 for normal rendering, 2 for double
/// buffering and 3 for triple buffering.
using FramesInFlight = BoundedU32<1, 3>;

/// @brief Parameters used to create a new @ref Swapchain.
struct SwapchainDescriptor {
    /// @brief If the swapchain should be created with vsync enabled.
    bool vsync;
    /// @brief The max number of frames in flight at a time.
    /// TODO: make this be used bro
    /// FramesInFlight frames_in_flight;
};

/// @brief Information about the swapchain that the user cannot define, but
/// may query the swapchain for.
struct SwapchainInfo {
    /// @brief The format of the pixels of the swapchains image.
    ImageFormat image_format;
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
    auto descriptor() const -> const SwapchainDescriptor&;

    /// @brief Returns the descriptor of this @ref Swapchain.
    [[nodiscard]]
    auto info() const -> SwapchainInfo;

    /// @brief Returns the next free image managed by this @ref Swapchain to
    /// render to.
    [[nodiscard]]
    auto next_image() const -> ImageHandle;

    /// @brief Presents the back buffer to the screen.
    auto present() const -> void;

    /// @brief Presents the back buffer to the screen after the commands have
    /// finished executing.
    auto present(CommandList&& cmds) const -> void;
};
} // namespace siren
