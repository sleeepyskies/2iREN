#pragma once

#include "resources/fwd.hpp"
#include "fwd.hpp"

namespace siren {

struct SwapchainDescriptor {
    std::optional<std::string> label;
    Window& window;
    Device& device;
    u8 buffer_count;
};

class Swapchain {
public:
    explicit Swapchain(const SwapchainDescriptor& descriptor);
    ~Swapchain();

    auto current_framebuffer() -> Image;

private:
    Window& m_window;
    Device& m_device;
    SwapchainDescriptor& m_descriptor;
    std::vector<Image> m_targets;
};

} // namespace siren
