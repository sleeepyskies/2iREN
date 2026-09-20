#include "commands.hpp"

#include <Foundation/Foundation.hpp>
#include <Metal/MTLResource.hpp>
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
    const u32 buffer_offset,
    const ByteBufferView data
) -> void {
    const auto& desc = m_state.buffers.details(buffer);

    ASSERT(desc.memory_usage != MemoryUsage::GpuOnly, "cannot upload to GpuOnly buffer.");
    ASSERT(
        desc.size.get() - buffer_offset >= data.size(),
        "buffer is too small to write the requested data."
    );

    auto mtlbuf = m_state.buffers.fetch(buffer);
    ASSERT_NOT_NULL(mtlbuf.get());

    bufcpy(data, mtlbuf->contents());
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

} // namespace siren::metal
