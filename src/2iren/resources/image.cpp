#include "image.hpp"

#include "2iren/device.hpp"


namespace siren {

Image::Image(
    Device* device,
    const ImageHandle handle
) : Base(device, handle) { }

Image::~Image() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_image(m_handle);
    }
}

Image::Image(Image&& other) noexcept
    : Base(std::move(other)) { }

Image& Image::operator=(Image&& other) noexcept {
    if (this != &other) {
        // cleanup old buffer
        if (m_device && m_handle.is_valid()) {
            m_device->destroy_image(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto Image::descriptor() const noexcept -> const ImageDescriptor& { return m_device->image_descriptor(m_handle); }

} // namespace siren
