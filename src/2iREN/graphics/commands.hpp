#pragma once

#include <functional>
#include <variant>

#include "2iREN/container/bytebuffer.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/statistics.hpp"
#include "2iREN/graphics/types.hpp"
#include "2iREN/math/color.hpp"
#include "2iREN/math/extent.hpp"
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
    Rgba clear_color               = Rgba::ZERO();
    BeginOperation begin_operation = BeginOperation::Clear;
    EndOperation end_operation     = EndOperation::Store;
};

using TargetColorAttachments = std::vector<TargetColorAttachment>;

struct TargetDepthStencilAttachment {
    ImageHandle image;
    f32 clear_depth                = 1.f;
    u32 clear_stencil              = 0u;
    BeginOperation begin_operation = BeginOperation::Clear;
    EndOperation end_operation     = EndOperation::Store;
};

struct RenderTarget {
    TargetColorAttachments colors                             = {};
    std::optional<TargetDepthStencilAttachment> depth_stencil = std::nullopt;
};

struct RenderTargetless {
    Extent2 extent;
};

using RenderTargetVariant = std::variant<RenderTarget, RenderTargetless>;

struct RenderPassDescriptor {
    Label label                = std::nullopt;
    RenderTargetVariant target = {};
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
    /// @param offset An optional offset in bytes to bind the vertex buffer starting from.
    virtual auto bind_vertex_buffer(BufferHandle buffer, Slot slot, u32 offset = 0) -> void = 0;

    /// @brief Binds the provided @ref Buffer as the active index buffer.
    /// @param buffer The index buffer to bind.
    /// @param type The type of the indices being bound.
    virtual auto bind_index_buffer(BufferHandle buffer, IndexType type) -> void = 0;

    /// @brief Binds the provided @ref Buffer as a uniform uniform at the provided slot.
    /// @param buffer The uniform buffer to bind.
    /// @param slot The binding slot.
    /// @param offset An optional offset in bytes to bind the vertex buffer starting from.
    virtual auto bind_uniform_buffer(BufferHandle buffer, Slot slot, u32 offset = 0) -> void = 0;

    /// @brief Binds the provided @ref Buffer as a storage uniform at the provided slot.
    /// @param buffer The storage buffer to bind.
    /// @param slot The binding slot.
    /// @param offset An optional offset in bytes to bind the vertex buffer starting from.
    virtual auto bind_storage_buffer(BufferHandle buffer, Slot slot, u32 offset = 0) -> void = 0;

    /// @brief Binds an image and sampler for filtered shader reads.
    /// @param image The sampled image to bind.
    /// @param sampler The sampler controlling how the image is sampled.
    /// @param slot The shared image/sampler binding slot.
    virtual auto bind_sampled_image(ImageHandle image, SamplerHandle sampler, Slot slot)
        -> void = 0;

    /// @brief Binds an image for direct shader load/store operations.
    /// @param image The storage image to bind.
    /// @param slot The storage image binding slot.
    virtual auto bind_storage_image(ImageHandle image, Slot slot) -> void = 0;

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

/// @brief Handles recording commands into a command buffer. Once commands
/// recording has finished, submit this to the device.
/// been finishe
class CommandBuffer {
public:
    explicit CommandBuffer() = default;
    virtual ~CommandBuffer() = default;

    virtual auto render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& encode)
        -> void = 0;

    /// @brief Fills a buffer with some value.
    /// @param buffer The buffer to fill.
    /// @param value The byte value to fill the buffer with.
    /// @param range An optional byte indexed range to write to. Defaults to the full buffer range
    ///        if no value is provided.
    virtual auto fill_buffer(BufferHandle buffer, u8 value, Range<usize> range = {}) -> void = 0;

    /// @brief Writes cpu data into a gpu buffer.
    /// @param dst The buffer to write to.
    /// @param dst_offset An offset in bytes to start writing to the dst buffer.
    /// @param data The cpu data to write to the gpu buffer.
    virtual auto write_buffer(BufferHandle dst, usize dst_offset, ByteBufferView data) -> void = 0;

    /// @brief Writes cpu data into a gpu image.
    /// @param dst The image to write to.
    /// @param data The cpu data to write to the gpu buffer.
    /// @param layer The image layer to write to. Defaults to 0.
    virtual auto write_image(ImageHandle dst, ByteBufferView data, u32 layer = 0) -> void = 0;

    /// @brief Copies data from one gpu stored buffer to another.
    /// @param src The buffer holding the data to copy.
    /// @param src_range The range of the source buffer to copy.
    /// @param dst The buffer the data should be written to.
    /// @param dst_offset The byte offset into the destination buffer the data should be written
    ///        from.
    virtual auto copy_buffer_to_buffer(
        BufferHandle src,
        Range<usize> src_range,
        BufferHandle dst,
        usize dst_offset
    ) -> void = 0;

    /// @brief Copies buffer data into an entire image.
    /// @param src The source buffer.
    /// @param src_offset The byte offset at which to begin reading from the source buffer.
    /// @param dst The destination image, whose extent and format determine the amount of data
    ///        copied.
    virtual auto copy_buffer_to_image(BufferHandle src, usize src_offset, ImageHandle dst)
        -> void = 0;

    /// @brief Copies an entire image into a buffer.
    /// @param src The source image, whose extent and format determine the amount of data copied.
    /// @param dst The destination buffer.
    /// @param dst_offset The byte offset at which to begin writing to the destination buffer.
    virtual auto copy_image_to_buffer(ImageHandle src, BufferHandle dst, usize dst_offset)
        -> void = 0;

    /// @brief Copies the contents of one image into another.
    /// @param src The source image.
    /// @param dst The destination image.
    virtual auto copy_image_to_image(ImageHandle src, ImageHandle dst) -> void = 0;

    [[nodiscard]]
    auto statistics() const noexcept -> const Statistics&;

protected:
    Statistics m_statistics = {};
};

} // namespace siren
