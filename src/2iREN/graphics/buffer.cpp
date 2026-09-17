#include "buffer.hpp"

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/commands.hpp"
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

auto Buffer::upload(const ByteBufferView, const u32) const noexcept -> void {
    UNIMPLEMENTED();
    /*
    auto cmds = m_device->make_command_recorder();
    cmds.transfer_pass({.label = std::nullopt}, [](TransferCommandRecorder& transfer) {
        transfer.upload_to_buffer(m_handle, data, offset);
    });
    m_device->submit(std::move(cmds).finish());
    */
}

} // namespace siren
