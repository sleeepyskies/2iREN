#include "buffer.hpp"

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/device.hpp"

namespace siren {

Buffer::Buffer(Device* device, const BufferHandle handle) : Base(device, handle) { }

Buffer::~Buffer() {
    if (m_device && m_handle.is_valid()) {
        m_device->destroy_buffer(m_handle);
    }
}

Buffer::Buffer(Buffer&& other) noexcept : Base(std::move(other)) { }

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

auto Buffer::write(const ByteBufferView data, const u32 offset) const noexcept -> void {
    auto cmds = m_device->make_command_buffer();
    cmds->write_buffer(m_handle, offset, data);
    m_device->submit(std::move(cmds));
}

auto Buffer::fill(const u8 value, const Range<usize> range) const noexcept -> void {
    auto cmds = m_device->make_command_buffer();
    cmds->fill_buffer(m_handle, value, range);
    m_device->submit(std::move(cmds));
}

} // namespace siren
