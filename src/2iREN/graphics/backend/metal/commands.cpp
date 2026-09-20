#include "commands.hpp"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/core/assert.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/backend/metal/mappings.hpp"
#include "2iREN/graphics/backend/metal/resource_state.hpp"
#include "2iREN/graphics/backend/metal/util.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/fwd.hpp"

namespace siren::metal {

namespace {

struct ImageCopyLayout {
    MTL::Size size;
    usize bytes_per_row;
    usize bytes_per_slice;
    usize slice_count;
};

auto image_copy_layout(MTL::Texture* texture, ImageFormat format) -> ImageCopyLayout {
    ASSERT(texture->sampleCount() == 1, "buffer/image copies require a single-sample image.");
    ASSERT(!texture->isFramebufferOnly(), "cannot copy a framebuffer-only image.");
    ASSERT(
        format != ImageFormat::Depth24Stencil8,
        "buffer/image copies of combined depth/stencil require an explicit plane selection."
    );

    const auto size = MTL::Size{texture->width(), texture->height(), texture->depth()};
    const auto bytes_per_row = size.width * format.bytes_per_pixel();
    const auto is_cube = texture->textureType() == MTL::TextureTypeCube
        || texture->textureType() == MTL::TextureTypeCubeArray;

    return {
        .size = size,
        .bytes_per_row = bytes_per_row,
        .bytes_per_slice = bytes_per_row * size.height * size.depth,
        .slice_count = texture->arrayLength() * (is_cube ? 6 : 1),
    };
}

} // namespace

auto RenderCommandEncoder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void {
    const auto& descriptor = m_state.pipelines.details(pipeline);
    auto* pipeline_state   = m_state.pipelines.fetch(pipeline).get();
    m_encoder->setRenderPipelineState(pipeline_state);

    m_bindings.pipeline = descriptor;

    // we set format, alpha mode, color and alpha blend during creation of the RenderPipelineState
    // so for now we can skip

    // depth stencil pipeline state
    if (descriptor.depth_stencil.has_value()) {
        auto ds_desc = transfer_ptr(MTL::DepthStencilDescriptor::alloc()->init());
        ds_desc->setDepthCompareFunction(
            compare_function(descriptor.depth_stencil->compare_function)
        );
        ds_desc->setDepthWriteEnabled(descriptor.depth_stencil->depth_write);

        auto ds_state = transfer_ptr(m_encoder->device()->newDepthStencilState(ds_desc.get()));
        m_encoder->setDepthStencilState(ds_state.get());
    }

    // cull mode and force always CCW winding
    m_encoder->setCullMode(cull_mode(descriptor.cull_mode));
    m_encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
}

auto RenderCommandEncoder::bind_vertex_buffer(
    const BufferHandle buffer,
    const u32 offset,
    const u32 slot
) -> void {
    ASSERT(
        m_state.buffers.details(buffer).usage.test(BufferFlag::Vertex),
        "buffer must have BufferFlag::Vertex to be bound as a vetex buffer."
    );
    auto buf = m_state.buffers.fetch(buffer);
    m_encoder->setVertexBuffer(buf.get(), offset, slot);
}

auto RenderCommandEncoder::bind_index_buffer(
    const BufferHandle buffer,
    const IndexFormat index_format
) -> void {
    ASSERT(
        m_state.buffers.details(buffer).usage.test(BufferFlag::Index),
        "buffer must have BufferFlag::Index to be bound as an index buffer."
    );
    m_bindings.index = BindIdxBuf{buffer, index_format};
}

auto RenderCommandEncoder::bind_uniform_buffer(
    const BufferHandle buffer,
    const u32 offset,
    const u32 slot
) -> void {
    ASSERT(
        m_state.buffers.details(buffer).usage.test(BufferFlag::Uniform),
        "buffer must have BufferFlag::Uniform to be bound as a uniform buffer."
    );
    // TODO: should we use setVertexBytes for uniform buffers instead?
    auto buf = m_state.buffers.fetch(buffer);
    m_encoder->setVertexBuffer(buf.get(), offset, slot);
}

auto RenderCommandEncoder::bind_storage_buffer(const BufferHandle buffer, const u32) -> void {
    ASSERT(
        m_state.buffers.details(buffer).usage.test(BufferFlag::Storage),
        "buffer must have BufferFlag::Storage to be bound as a storage buffer."
    );
    UNIMPLEMENTED();
}

auto RenderCommandEncoder::bind_image(const ImageHandle image, const u32 slot) -> void {
    // TODO: do we want this in the fragment too/instead?
    // TODO: do we want to bind always to all stages?
    auto img = m_state.images.fetch(image).get();
    m_encoder->setVertexTexture(img, slot);
    m_encoder->setFragmentTexture(img, slot);
}

auto RenderCommandEncoder::bind_sampler(const SamplerHandle sampler, const u32 slot) -> void {
    // TODO: do we want to bind always to all stages?
    auto* ss = m_state.samplers.fetch(sampler).get();
    m_encoder->setVertexSamplerState(ss, slot);
    m_encoder->setFragmentSamplerState(ss, slot);
}

auto RenderCommandEncoder::draw_arrays(const u32 start, const u32 count) -> void {
    m_encoder->drawPrimitives(primitive_type(m_bindings.pipeline->topology), start, count);
}

auto RenderCommandEncoder::draw_indexed(const u32 index_count, const u32 first_index) -> void {
    auto* buf = m_state.buffers.fetch(m_bindings.index->buffer).get();

    m_encoder->drawIndexedPrimitives(
        primitive_type(m_bindings.pipeline->topology),
        index_count,
        index_type(m_bindings.index->format),
        buf,
        m_bindings.index->format.size_bytes() * first_index
    );
}

auto CommandBuffer::render_pass(
    const RenderPassDescriptor& descriptor,
    RenderPassFunction&& function
) -> void {
    auto desc = transfer_ptr(MTL::RenderPassDescriptor::alloc()->init());

    // setup color attachments
    for (usize i = 0; i < descriptor.target.colors.size(); i++) {
        auto& attachment     = descriptor.target.colors[i];
        auto* mtl_attachment = desc->colorAttachments()->object(i);
        auto mtl_texture     = m_state.images.fetch(attachment.image);

        const auto& rgba = attachment.clear_color;

        mtl_attachment->setTexture(mtl_texture.get());
        mtl_attachment->setClearColor(MTL::ClearColor::Make(rgba.r, rgba.g, rgba.b, rgba.a));
        mtl_attachment->setLoadAction(metal::load_action(attachment.begin_operation));
        mtl_attachment->setStoreAction(metal::store_action(attachment.end_operation));
    }

    // setup depth stencil
    if (descriptor.target.depth_stencil) {
        auto& attachment = *descriptor.target.depth_stencil;
        auto* depth      = desc->depthAttachment();
        auto depth_txt   = m_state.images.fetch(attachment.image);
        auto* stencil    = desc->stencilAttachment();
        auto stencil_txt = m_state.images.fetch(attachment.image);

        depth->setTexture(depth_txt.get());
        depth->setLoadAction(metal::load_action(attachment.begin_operation));
        depth->setClearDepth(attachment.clear_depth);
        depth->setStoreAction(metal::store_action(attachment.end_operation));

        stencil->setTexture(stencil_txt.get());
        stencil->setLoadAction(metal::load_action(attachment.begin_operation));
        stencil->setClearStencil(attachment.clear_stencil);
        stencil->setStoreAction(metal::store_action(attachment.end_operation));
    }

    auto mtlencoder     = transfer_ptr(m_cmdbuffer->renderCommandEncoder(desc.get()));
    auto render_encoder = RenderCommandEncoder{m_state, mtlencoder};
    std::invoke(function, render_encoder);

    mtlencoder->endEncoding();
}

auto CommandBuffer::write_buffer(
    const BufferHandle buffer,
    const usize buffer_offset,
    const ByteBufferView data
) -> void {
    const auto& desc = m_state.buffers.details(buffer);

    ASSERT(desc.memory_usage != MemoryUsage::GpuOnly, "cannot upload to GpuOnly buffer.");
    ASSERT(
        buffer_offset <= desc.size.get() && data.size() <= desc.size.get() - buffer_offset,
        "buffer is too small to write the requested data."
    );

    auto mtlbuf = m_state.buffers.fetch(buffer);
    ASSERT_NOT_NULL(mtlbuf.get());

    bufcpy(data, static_cast<u8*>(mtlbuf->contents()) + buffer_offset);
}

auto CommandBuffer::fill_buffer(const BufferHandle buffer, const RangeUsize range, const u8 value)
    -> void {
    // TODO: should we really make a new blit command encoder per upload?
    AUTORELEASE {
        const auto& desc = m_state.buffers.details(buffer);
        ASSERT(desc.memory_usage != MemoryUsage::GpuOnly, "cannot upload to GpuOnly buffer.");
        ASSERT(
            desc.size.get() >= range.length(),
            "buffer is not large enough to write the requested amount of data."
        );
        auto mtlbuf = m_state.buffers.fetch(buffer).get();

        auto encoder = m_cmdbuffer->blitCommandEncoder();
        encoder->fillBuffer(mtlbuf, NS::Range(range.begin, range.length()), value);
        encoder->endEncoding();
    }
}

auto CommandBuffer::write_image(const ImageHandle image, const ByteBufferView data) -> void {
    const auto& desc = m_state.images.details(image);

    ASSERT(desc.memory_usage != MemoryUsage::GpuOnly, "cannot upload to GpuOnly image.");
    ASSERT(desc.extent.area() >= data.size(), "image is too small to write the requested data.");

    auto mtlimg = m_state.images.fetch(image);
    ASSERT_NOT_NULL(mtlimg.get());

    const auto bytes_per_row = desc.extent.x * desc.format.bytes_per_pixel();
    mtlimg->replaceRegion(region(desc.extent), 0, data.data(), bytes_per_row);
}

auto CommandBuffer::copy_buffer_to_buffer(
    BufferHandle src,
    RangeUsize src_range,
    BufferHandle dst,
    usize dst_offset
) -> void {
    AUTORELEASE {
        auto mtlsrc   = m_state.buffers.fetch(src).get();
        auto mtldst   = m_state.buffers.fetch(dst).get();
        auto* encoder = m_cmdbuffer->blitCommandEncoder();
        encoder->copyFromBuffer(mtlsrc, src_range.begin, mtldst, dst_offset, src_range.length());
        encoder->endEncoding();
    }
}

auto CommandBuffer::copy_buffer_to_image(
    BufferHandle src,
    usize src_offset,
    ImageHandle dst
) -> void {
    AUTORELEASE {
        auto* mtlsrc = m_state.buffers.fetch(src).get();
        auto* mtldst = m_state.images.fetch(dst).get();
        const auto format = m_state.images.details(dst).format;
        const auto layout = image_copy_layout(mtldst, format);
        const auto size_bytes = layout.bytes_per_slice * layout.slice_count;

        ASSERT(src_offset % format.bytes_per_pixel() == 0, "unaligned source buffer offset.");
        ASSERT(
            src_offset <= mtlsrc->length() && size_bytes <= mtlsrc->length() - src_offset,
            "source buffer is too small to copy the image."
        );

        auto* encoder = m_cmdbuffer->blitCommandEncoder();
        for (usize slice = 0; slice < layout.slice_count; ++slice) {
            encoder->copyFromBuffer(
                mtlsrc,
                src_offset + slice * layout.bytes_per_slice,
                layout.bytes_per_row,
                layout.size.depth > 1 ? layout.bytes_per_row * layout.size.height : 0,
                layout.size,
                mtldst,
                slice,
                0,
                MTL::Origin{0, 0, 0}
            );
        }
        encoder->endEncoding();
    }
}

auto CommandBuffer::copy_image_to_buffer(
    ImageHandle src,
    BufferHandle dst,
    usize dst_offset
) -> void {
    AUTORELEASE {
        auto* mtlsrc = m_state.images.fetch(src).get();
        auto* mtldst = m_state.buffers.fetch(dst).get();
        const auto format = m_state.images.details(src).format;
        const auto layout = image_copy_layout(mtlsrc, format);
        const auto size_bytes = layout.bytes_per_slice * layout.slice_count;

        ASSERT(dst_offset % format.bytes_per_pixel() == 0, "unaligned destination buffer offset.");
        ASSERT(
            dst_offset <= mtldst->length() && size_bytes <= mtldst->length() - dst_offset,
            "destination buffer is too small to copy the image."
        );

        auto* encoder = m_cmdbuffer->blitCommandEncoder();
        for (usize slice = 0; slice < layout.slice_count; ++slice) {
            encoder->copyFromTexture(
                mtlsrc,
                slice,
                0,
                MTL::Origin{0, 0, 0},
                layout.size,
                mtldst,
                dst_offset + slice * layout.bytes_per_slice,
                layout.bytes_per_row,
                layout.size.depth > 1 ? layout.bytes_per_row * layout.size.height : 0
            );
        }
        encoder->endEncoding();
    }
}

auto CommandBuffer::copy_image_to_image(ImageHandle src, ImageHandle dst) -> void {
    AUTORELEASE {
        auto* mtlsrc = m_state.images.fetch(src).get();
        auto* mtldst = m_state.images.fetch(dst).get();

        ASSERT(mtlsrc->pixelFormat() == mtldst->pixelFormat(), "image formats must match.");
        ASSERT(mtlsrc->textureType() == mtldst->textureType(), "image dimensions must match.");
        ASSERT(
            mtlsrc->width() == mtldst->width() && mtlsrc->height() == mtldst->height()
                && mtlsrc->depth() == mtldst->depth() && mtlsrc->arrayLength() == mtldst->arrayLength(),
            "image extents and slice counts must match."
        );
        ASSERT(mtlsrc->sampleCount() == mtldst->sampleCount(), "image sample counts must match.");
        ASSERT(
            !mtlsrc->isFramebufferOnly() && !mtldst->isFramebufferOnly(),
            "cannot copy a framebuffer only image."
        );

        const auto is_cube = mtlsrc->textureType() == MTL::TextureTypeCube
            || mtlsrc->textureType() == MTL::TextureTypeCubeArray;
        const auto slice_count = mtlsrc->arrayLength() * (is_cube ? 6 : 1);

        auto* encoder = m_cmdbuffer->blitCommandEncoder();
        encoder->copyFromTexture(mtlsrc, 0, 0, mtldst, 0, 0, slice_count, 1);
        encoder->endEncoding();
    }
}

} // namespace siren::metal
