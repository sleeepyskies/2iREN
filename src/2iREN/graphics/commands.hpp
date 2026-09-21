#pragma once

#include <functional>

#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/statistics.hpp"
#include "2iREN/graphics/types.hpp"
#include "2iREN/math/color.hpp"
#include "2iREN/math/range.hpp"

namespace siren {

class Device;

/// The operation to perform on an attachment when a pass begins.
enum class BeginOperation : u8 {
    /// Clear the attachment to its specified clear value.
    Clear,
    /// Preserve the attachment's existing contents.
    Preserve,
    /// Discard the attachment's existing contents.
    Fuckit,
};

/// The operation to perform on an attachment when a pass ends.
enum class EndOperation : u8 {
    /// Preserve the results for subsequent use.
    Store,
    /// Allow the results to be discarded.
    Fuckit,
};

struct TargetColorAttachment {
    ImageHandle image;
    Rgba clear_color;
    BeginOperation begin_operation = BeginOperation::Clear;
    EndOperation end_operation     = EndOperation::Store;
};

using TargetColorAttachments = std::vector<TargetColorAttachment>;

struct TargetDepthStenctilAttachment {
    ImageHandle image;
    f32 clear_depth;
    u32 clear_stencil;
    BeginOperation begin_operation = BeginOperation::Clear;
    EndOperation end_operation     = EndOperation::Store;
};

struct RenderTarget {
    TargetColorAttachments colors                              = {};
    std::optional<TargetDepthStenctilAttachment> depth_stencil = std::nullopt;
};

struct RenderPassDescriptor {
    Label label = std::nullopt;
    RenderTarget target;
};

/// @brief Value holding a slot binding.
struct Slot {
    u8 value;
};

/// @brief Handles recoding commands into the command buffer that relate to
/// rendering.
class RenderCommandEncoder {
public:
    virtual ~RenderCommandEncoder() = default;

    /// @brief Binds the provided @ref GraphicsPipeline.
    /// @param pipeline The pipeline to bind.
    virtual auto bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void = 0;

    /// @brief Binds the provided @ref Buffer as a vertex buffer at the provided slot.
    /// @param buffer The vertex buffer to bind.
    /// @param slot The binding slot.
    /// @param range An optional range of the buffer to bind. A value of
    ///        Range::full() will bind the whole buffer.
    virtual auto bind_vertex_buffer(BufferHandle buffer, Slot slot, Range<usize> range = {})
        -> void = 0;

    /// @brief Binds the provided @ref Buffer as the active index buffer.
    /// @param buffer The index buffer to bind.
    /// @param type The type of the indices being bound.
    virtual auto bind_index_buffer(BufferHandle buffer, IndexType type) -> void = 0;

    /// @brief Binds the provided @ref Buffer as a uniform uniform at the provided slot.
    /// @param buffer The uniform buffer to bind.
    /// @param slot The binding slot.
    /// @param range An optional range of the buffer to bind. A value of
    ///        Range::full() will bind the whole buffer. Indexed as indices and
    ///        not as bytes.
    virtual auto bind_uniform_buffer(BufferHandle buffer, Slot slot, Range<usize> range = {})
        -> void = 0;

    /// @brief Binds the provided @ref Buffer as a storage uniform at the provided slot.
    /// @param buffer The storage buffer to bind.
    /// @param slot The binding slot.
    /// @param range An optional range of the buffer to bind. A value of
    ///        Range::full() will bind the whole buffer.
    virtual auto bind_storage_buffer(BufferHandle buffer, Slot slot, Range<usize> range = {})
        -> void = 0;

    /// @brief Binds the provided @ref Image at the provided slot.
    /// @param image The image to bind.
    /// @param slot The binding slot.
    virtual auto bind_image(ImageHandle image, Slot slot) -> void = 0;

    /// @brief Binds the provided @ref Sampler at the provided slot.
    /// @param sampler The sampler to bind.
    /// @param slot The binding slot.
    virtual auto bind_sampler(SamplerHandle sampler, Slot slot) -> void = 0;

    /// @brief Draws primitves based on the currently bound vertex buffer.
    /// @param count The number of vertices to draw.
    /// @param start An offset into the vertex buffer in vertices to begin drawing from.
    /// @param instance_count The number of instances to draw.
    /// @param instance_start The first instance to draw.
    virtual auto draw(u32 count, u32 start = 0, u32 instance_count = 1, u32 instance_start = 0)
        -> void = 0;

    /// @brief Draws primitves based on the currently bound vertex and index buffer.
    /// @brief count The number of vertices to read from the index buffer.
    /// @brief start The first index of the index buffer to read from.
    virtual auto draw_indexed(u32 count, u32 start = 0) -> void = 0;
};

using RenderPassFunction = std::function<void(RenderCommandEncoder&)>;

class CommandBuffer {
public:
    explicit CommandBuffer() = default;
    virtual ~CommandBuffer() = default;

    virtual auto render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& encode)
        -> void = 0;

    virtual auto fill_buffer(BufferHandle buffer, u8 value, Range<usize> range = {}) -> void = 0;

    virtual auto write_buffer(BufferHandle dest, usize dest_offset, ByteBufferView data)
        -> void                                                             = 0;
    virtual auto write_image(ImageHandle dest, ByteBufferView data) -> void = 0;

    virtual auto copy_buffer_to_buffer(
        BufferHandle src,
        Range<usize> src_range,
        BufferHandle dest,
        usize dest_offset
    ) -> void = 0;
    virtual auto copy_buffer_to_image(BufferHandle src, usize src_offset, ImageHandle dst)
        -> void = 0;
    virtual auto copy_image_to_buffer(ImageHandle src, BufferHandle dst, usize dst_offset)
        -> void                                                                = 0;
    virtual auto copy_image_to_image(ImageHandle src, ImageHandle dst) -> void = 0;

    [[nodiscard]]
    virtual auto statistics() const -> const Statistics& {
        return m_statistics;
    }

protected:
    Statistics m_statistics = {};
};

} // namespace siren
