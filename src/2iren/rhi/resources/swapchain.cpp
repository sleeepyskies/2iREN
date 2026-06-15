#include "swapchain.hpp"

#include "2iren/rhi/device.hpp"
#include "2iren/window.hpp"


namespace siren {

Swapchain::Swapchain(
    Device* device,
    const SwapchainHandle handle
) : Base(device, handle) { }

Swapchain::~Swapchain() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_swapchain(m_handle);
    }
}

Swapchain::Swapchain(Swapchain&& other) noexcept : Base(std::move(other)) { }

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
    if (this != &other) {
        // cleanup old buffer
        if (m_device && m_handle.is_valid()) {
            m_device->destroy_swapchain(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto Swapchain::descriptor() const -> const SwapchainDescriptor& { return m_device->swapchain_descriptor(m_handle); }

auto Swapchain::current_framebuffer() const -> FramebufferHandle {
    return m_device->acquire_next_swapchain_target(m_handle);
}

auto Swapchain::present() const -> void {
    m_device->present(m_handle);
}

} // namespace siren
