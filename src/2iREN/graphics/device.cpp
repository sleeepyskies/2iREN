#include "device.hpp"

#define VALIDATION if (validate())

namespace siren {

Device::Device(ValidationMode validation) : m_validation(validation) { }

/*
auto Device::make_buffer(
    const BufferDescriptor& descriptor,
    std::optional<ByteBufferView> initial
) -> Buffer {
    VALIDATION {
        if (initial.has_value()) {
            ASSERT(initial->size() == descriptor.size.get(), "initial buffer data must exactly match the requested buffer size");
        }
    }

    auto buffer = make_buffer_impl(descriptor, initial);
    if (buffer.handle() != NullHandle) {
        log::trace("made buffer {}", buffer);
    } else {
        log::warn("failed to make buffer");
    }
    return buffer;
}

auto Device::invalid_buffer() -> Buffer {
    return Buffer{ this, NullHandle };
}

*/

}
