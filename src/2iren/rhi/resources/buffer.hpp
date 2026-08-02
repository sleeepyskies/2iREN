#pragma once

#include <libassert/assert.hpp>
#include <vector>

#include "2iren/base.hpp"
#include "2iren/util/byte_buffer.hpp"
#include "fwd.hpp"

namespace siren {
class ByteBuffer;

/**
 * @brief Defines the usage of a Buffer.
 */
enum class BufferUsage {
    /** @brief Best used for data that is set once and persistent such as static meshes. */
    Static,
    /** @brief Best used for data is updated often and persistent like dynamic meshes. */
    Dynamic,
    /** @brief Best used for data that is updated often but not persistent like UI or short term effects. */
    Stream,
};

/** @brief Defines the index format of an index buffer. */
class IndexFormat {
public:
    enum Value : u8 {
        UInt8,
        UInt16,
        UInt32,
    } value;

    /** @brief Returns the size of this format in bytes. */
    [[nodiscard]] constexpr auto size_bytes() const -> usize {
        switch (value) {
            case UInt8: return 1;
            case UInt16: return 2;
            case UInt32: return 4;
            default: UNREACHABLE();
        }
    }

    IndexFormat(const Value v) : value(v) {}
    constexpr operator Value() const { return value; }
};

/**
 * @brief Describes a @ref Buffer. Used for object creation via @ref Device.
 * @todo pass in the data as a separate buffer in the device->create_buffer(), or just remove entirely
 */
struct BufferDescriptor {
    /** @brief An optional label. Mainly useful for debugging. */
    std::optional<std::string> label = std::nullopt;
    /**
     * @brief Optional initial data. If present, performs a direct upload.
     * @note This is dropped by the @ref Buffer once owned by it. Therefore,
     * it is best to never access this via a @ref Buffer
     * @todo remove this? idk
     * @todo should this field actually be a @ref ByteBuffer instead?
     */
    std::optional<std::vector<u8>> data = std::nullopt;
    /** @brief The initial size of the buffer in bytes. */
    usize size;
    /** @brief The intended use of the buffer. */
    BufferUsage usage;
};

/**
 * @brief The Buffer class represents a chunk of GPU memory.
 * This makes use of OpenGL DSA and can thus be used as a buffer
 * for arbitrary types of data (vertex buffers, index buffers etc...)
 */
class Buffer final : public RenderResource<Buffer> {
public:
    using Base = RenderResource<Buffer>;

    Buffer(Device* device, BufferHandle handle);
    ~Buffer();
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    /** @brief Returns the descriptor of this Buffer. */
    [[nodiscard]] auto descriptor() const noexcept -> const BufferDescriptor&;

    /** @brief Utility function to upload data to this Buffer. */
    auto upload(const ByteBuffer& data, const u32 offset = 0) const noexcept -> void;

    /** @brief Utility function to upload data to this Buffer. */
    template <typename T>
    auto upload(const T& data) const noexcept -> void { upload(ByteBuffer{data}, 0); }
};
} // namespace siren
