#include "framebuffer.hpp"

#include "2iren/device.hpp"


namespace siren
{
Framebuffer::Framebuffer(
    Device* device,
    const FramebufferHandle handle,
    std::vector<Image>&& colors,
    std::optional<Image>&& depth_stencil
) : Base(device, handle),
    m_colors(std::move(colors)),
    m_depth_stencil(std::move(depth_stencil)) { }

Framebuffer::~Framebuffer() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_framebuffer(m_handle);
    }
}

Framebuffer::Framebuffer(Framebuffer&& other) noexcept
    : Base(std::move(other)),
      m_colors(std::move(other.m_colors)) { }

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
    if (this != &other) {
        // cleanup old buffer
        if (m_device && m_handle.is_valid()) {
            m_device->destroy_framebuffer(m_handle);
        }

        Base::operator=(std::move(other));

        m_colors        = std::move(other.m_colors);
        m_depth_stencil = std::move(other.m_depth_stencil);
    }
    return *this;
}

auto Framebuffer::descriptor() const noexcept -> const FramebufferDescriptor& {
    return m_device->framebuffer_descriptor(m_handle);
}

auto Framebuffer::color_attachment(const usize index) const noexcept -> const Image* {
    if (index > m_colors.size()) { return nullptr; }
    return &m_colors[index];
}

auto Framebuffer::depth_stencil_attachment() const noexcept -> const Image* {
    if (m_depth_stencil.has_value()) { return &m_depth_stencil.value(); }
    return nullptr;
}

} // namespace siren
