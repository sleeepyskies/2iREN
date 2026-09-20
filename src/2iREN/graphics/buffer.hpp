#pragma once

#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/container/flag_set.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/types.hpp"
#include "2iREN/math/bounded.hpp"

namespace siren {

class ByteBuffer;

/// @brief Defines the usage of a Buffer.
enum class BufferFlag {
    /// @brief The buffer may be used as a vertex buffer.
    Vertex,
    /// @brief The buffer may be used as an index buffer.
    Index,
    /// @brief The buffer may be used as a uniform buffer for smallish read only data.
    Uniform,
    /// @brief The buffer may be used as a storage buffer for large read-write data.
    Storage,

    Max,
};

/// @brief Set of flags defining how a buffer may be used.
using BufferFlags = FlagSet<BufferFlag>;

/// @brief Describes a @ref Buffer. Used for object creation via @ref Device.
/// @todo pass in the data as a separate buffer in the device->create_buffer(),
/// or just remove entirely
struct BufferDescriptor {
    /// @brief An optional label.
    Label label = std::nullopt;
    /// @brief The initial size of the buffer in bytes.
    NonZeroUsize size;
    /// @brief Flag set of this buffers uses.
    BufferFlags usage;
    /// @brief Denotes what components may access the buffer.
    MemoryUsage memory_usage;
};

/// @brief A Buffer represents a typeless memory allocation on the GPU. The
/// app defines how the memory is laid out. Can be used to represent various
/// things such as vertex buffers, index buffers, shader storage objects,
/// uniform buffers. The main use of buffers is to be used from shaders.
class Buffer final : public RenderResource<Buffer> {
public:
    using Base = RenderResource<Buffer>;

    Buffer(Device* device, BufferHandle handle);
    ~Buffer();
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    /// @brief Returns the descriptor of this Buffer.
    [[nodiscard]]
    auto descriptor() const noexcept -> const BufferDescriptor&;

    /// @brief Utility function to upload data to this Buffer.
    auto upload(const ByteBufferView data, const u32 offset = 0) const noexcept -> void;
};
} // namespace siren
