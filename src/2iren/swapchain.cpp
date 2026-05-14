#include "swapchain.hpp"

#include "device.hpp"
#include "window.hpp"
#include "resources/image.hpp"

namespace siren {

/** @brief Simple helper to create a label for an @ref image. */
static auto create_label(const std::optional<std::string> label, const u32 idx) -> std::optional<std::string> {
    return label.transform([idx](const std::string& l){
        return l + "_image_" + std::to_string(idx);
    });
}

Swapchain::Swapchain(const SwapchainDescriptor& descriptor) : m_window(descriptor.window), m_device(descriptor.device) {
    m_targets.reserve(descriptor.buffer_count);
    for (const u32 idx : range(descriptor.buffer_count)) {
        m_targets.push_back(m_device.create_image({
            .label = create_label(descriptor.label, idx),
            .format = , // todo: what format?
            .extent = {
                .width = m_descriptor.window.width(),
                .height = m_descriptor.window.height(),
                .depth_or_layers = 1,
            },
            .dimension = ImageDimension::D2,
            .mipmap_levels = 1, // todo: idk
        }));
    }
}

Swapchain::~Swapchain() { }

auto Swapchain::next_image() -> Image { }

} // namespace siren
