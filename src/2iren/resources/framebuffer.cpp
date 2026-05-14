#include "framebuffer.hpp"

#include "2iren/device.hpp"


namespace siren {
Framebuffer::Framebuffer(
    Device* device,
    const FramebufferHandle handle
) : Base(device, handle) { }

Framebuffer::~Framebuffer() {
    if (m_device && m_handle.valid()) {
        m_device->destroy_framebuffer(m_handle);
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : Base(std::move(other)) { }

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        // cleanup old buffer
        if (m_device && m_handle.valid()) {
            m_device->destroy_framebuffer(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto Framebuffer::descriptor() const noexcept -> const FramebufferDescriptor& {
    return m_device->framebuffer_descriptor(m_handle);
}

auto Framebuffer::color_attachment(const usize index) const noexcept -> const Image* {
    const auto& colors = m_device->framebuffer_attachments(m_handle).colors;
    if (index > colors.size()) { return nullptr; }
    return &colors[index];
}

auto Framebuffer::color_attachments() const noexcept -> const std::vector<Image>& {
    return m_device->framebuffer_attachments(m_handle).colors;
}

auto Framebuffer::depth_stencil_attachment() const noexcept -> const Image* {
    const auto& depth_stencil = m_device->framebuffer_attachments(m_handle).depth_stencil;
    if (!depth_stencil.has_value()) { return nullptr; }
    return &depth_stencil.value();
}

} // namespace siren
