#pragma once

#include <vector>
#include <libassert/assert.hpp>

#include "fwd.hpp"
#include "2iren/base.hpp"


namespace siren {

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
    enum Enum : u8 { Byte8, Short16, Uint32 } value;

    /** @brief Returns the size of this format in bytes. */
    [[nodiscard]] constexpr auto size_bytes() const -> usize {
        switch (value) {
            case Byte8: return 1;
            case Short16: return 2;
            case Uint32: return 4;
            default: UNREACHABLE();
        }
    }

    IndexFormat(const Enum v) : value(v) { } // NOLINT(*-explicit-constructor)
    constexpr operator Enum() const { return value; } // NOLINT(*-explicit-constructor)
};

/**
 * @brief Describes a @ref Buffer. Used for object creation via @ref Device.
 */
struct BufferDescriptor {
    /** @brief An optional label. Mainly useful for debugging. */
    std::optional<std::string> label;
    /**
     * @brief Optional initial data. If present, performs a direct upload.
     * @note This is dropped by the @ref Buffer once owned by it. Therefore,
     * it is best to never access this via a @ref Buffer
     * @todo remove this? idk
     */
    std::optional<std::vector<u8>> data;
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
};

} // namespace siren
