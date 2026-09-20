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

struct RenderPassColorAttachment {
    ImageHandle image;
    Rgba clear_color;
    BeginOperation begin_operation;
    EndOperation end_operation;
};

using RenderPassColorAttachments = std::vector<RenderPassColorAttachment>;

struct RenderPassDepthStenctilAttachment {
    ImageHandle image;
    f32 clear_depth;
    u32 clear_stencil;
    BeginOperation begin_operation;
    EndOperation end_operation;
};

struct RenderTarget {
    RenderPassColorAttachments colors;
    std::optional<RenderPassDepthStenctilAttachment> depth_stencil;
};

struct RenderPassDescriptor {
    Label label = std::nullopt;
    RenderTarget target;
};

class RenderCommandEncoder {
public:
    virtual ~RenderCommandEncoder() = default;

    virtual auto bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void = 0;

    virtual auto bind_vertex_buffer(BufferHandle buffer, u32 offset, u32 slot) -> void = 0;

    virtual auto bind_index_buffer(BufferHandle buffer, IndexFormat index_format) -> void = 0;

    virtual auto bind_uniform_buffer(BufferHandle buffer, u32 offset, u32 slot) -> void = 0;

    virtual auto bind_storage_buffer(BufferHandle buffer, u32 slot) -> void = 0;

    virtual auto bind_image(ImageHandle image, u32 slot) -> void = 0;

    virtual auto bind_sampler(SamplerHandle sampler, u32 slot) -> void = 0;

    virtual auto draw_arrays(u32 start, u32 count) -> void = 0;

    virtual auto draw_indexed(u32 index_count, u32 first_index) -> void = 0;
};

using RenderPassFunction = std::function<void(RenderCommandEncoder&)>;

class CommandBuffer {
public:
    explicit CommandBuffer() = default;
    virtual ~CommandBuffer() = default;

    virtual auto render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& encode)
        -> void = 0;

    virtual auto fill_buffer(BufferHandle buffer, RangeUsize range, u8 value) -> void = 0;

    virtual auto write_buffer(BufferHandle dest, usize dest_offset, ByteBufferView data)
        -> void                                                             = 0;
    virtual auto write_image(ImageHandle dest, ByteBufferView data) -> void = 0;

    virtual auto copy_buffer_to_buffer(
        BufferHandle src,
        RangeUsize src_range,
        BufferHandle dest,
        usize dest_offset
    ) -> void = 0;
    virtual auto copy_buffer_to_image(BufferHandle src, usize src_offset, ImageHandle dst)
        -> void = 0;
    virtual auto copy_image_to_buffer(ImageHandle src, BufferHandle dst, usize dst_offset)
        -> void = 0;
    virtual auto copy_image_to_image(ImageHandle src, ImageHandle dst) -> void = 0;

    [[nodiscard]]
    virtual auto statistics() const -> const Statistics& {
        return m_statistics;
    }

protected:
    Statistics m_statistics = {};
};

} // namespace siren
