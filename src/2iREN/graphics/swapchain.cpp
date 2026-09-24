#include "swapchain.hpp"

#include "2iREN/graphics/device.hpp"

namespace siren {

Swapchain::Swapchain(Device* device, const SwapchainHandle handle) : Base(device, handle) { }

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

auto Swapchain::info() const -> const SwapchainInfo& {
    return m_device->swapchain_info(m_handle);
}

auto Swapchain::reconfigure(const SwapchainDescriptor& new_values) -> void {
    m_device->reconfigure_swapchain(m_handle, new_values);
}

auto Swapchain::next_image() const -> ImageHandle {
    return m_device->acquire_next_swapchain_image(m_handle);
}

auto Swapchain::present() const -> void {
    m_device->present(m_handle);
}

auto Swapchain::present(std::unique_ptr<CommandBuffer>&& command_buffer) const -> void {
    m_device->present(m_handle, std::move(command_buffer));
}

} // namespace siren
