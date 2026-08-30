#include "buffer.hpp"

#include "2iREN/graphics/device.hpp"

namespace siren {

Buffer::Buffer(Device* device, const BufferHandle handle) : Base(device, handle) {}

Buffer::~Buffer() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_buffer(m_handle);
    }
}

Buffer::Buffer(Buffer&& other) noexcept : Base(std::move(other)) {}

Buffer& Buffer::operator=(Buffer&& other) noexcept {
    if (this != &other) {
        // cleanup old buffer
        if (m_device && m_handle.is_valid()) {
            m_device->destroy_buffer(m_handle);
        }

        Base::operator=(std::move(other));
    }
    return *this;
}

auto Buffer::descriptor() const noexcept -> const BufferDescriptor& {
    return m_device->buffer_descriptor(m_handle);
}

auto Buffer::upload(const ByteBuffer& data, const u32 offset) const noexcept -> void {
    m_device->resource_submit([this, &data, offset](ResourceCommandRecorder& cmds) -> void {
        cmds.upload_to_buffer(m_handle, data, offset);
    });
}

} // namespace siren
