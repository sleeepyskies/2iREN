#include "command_executor.hpp"

#include <Foundation/Foundation.hpp>
#include <Metal/MTLBlitCommandEncoder.hpp>
#include <Metal/MTLDepthStencil.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <cstring>

#include "2iREN/core/assert.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/backend/metal/device.hpp"
#include "2iREN/graphics/backend/metal/mappings.hpp"
#include "2iREN/graphics/backend/metal/util.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"

namespace siren {

MetalCommandExecutor::MetalCommandExecutor(
    MetalDeviceState&                 state,
    NS::SharedPtr<MTL::CommandBuffer> cmd_buffer
) : m_state(state), m_cmd_buffer(cmd_buffer) { }

auto MetalCommandExecutor::execute(CommandList&& cmds) -> void {
    for (auto&& pass : cmds.passes) {
        switch (pass.kind) {
            case CommandList::Pass::Kind::Render: {
                execute_render_pass(
                    std::move(pass.descriptor.render_descriptor),
                    cmds.command_view(pass.command_range)
                );
                break;
            }
            case CommandList::Pass::Kind::Transfer: {
                execute_transfer_pass(
                    std::move(pass.descriptor.transfer_descriptor),
                    cmds.command_view(pass.command_range)
                );
                break;
            }
        }
    }
}

auto MetalCommandExecutor::execute_render_pass(
    RenderPassDescriptor&&         descriptor,
    const std::span<const Command> cmds
) -> void {
    auto desc = metal::transfer_ptr(MTL::RenderPassDescriptor::alloc()->init());

    // setup color attachments
    for (usize i = 0; i < descriptor.target.colors.size(); i++) {
        auto& attachment     = descriptor.target.colors[i];
        auto* mtl_attachment = desc->colorAttachments()->object(i);
        auto* mtl_texture    = m_state.images.fetch(attachment.image);

        const auto& rgba = attachment.clear_color;

        mtl_attachment->setTexture(mtl_texture);
        mtl_attachment->setClearColor(MTL::ClearColor::Make(rgba.r, rgba.g, rgba.b, rgba.a));
        mtl_attachment->setLoadAction(metal::load_action(attachment.begin_operation));
        mtl_attachment->setStoreAction(metal::store_action(attachment.end_operation));
    }

    // setup depth stencil
    if (descriptor.target.depth_stencil) {
        auto& attachment  = *descriptor.target.depth_stencil;
        auto* depth       = desc->depthAttachment();
        auto* depth_txt   = m_state.images.fetch(attachment.image);
        auto* stencil     = desc->stencilAttachment();
        auto* stencil_txt = m_state.images.fetch(attachment.image);

        depth->setTexture(depth_txt);
        depth->setLoadAction(metal::load_action(attachment.begin_operation));
        depth->setClearDepth(attachment.clear_depth);
        depth->setStoreAction(metal::store_action(attachment.end_operation));

        stencil->setTexture(stencil_txt);
        stencil->setLoadAction(metal::load_action(attachment.begin_operation));
        stencil->setClearStencil(attachment.clear_stencil);
        stencil->setStoreAction(metal::store_action(attachment.end_operation));
    }

    auto encoder = m_cmd_buffer->renderCommandEncoder(desc.get());

    for (const auto& cmd : cmds) {
        switch (cmd.type) {
            case CommandKind::BindGraphicsPipeline: {
                bind_graphics_pipeline(encoder, cmd.as<BindGraphicsPipeline>());
                break;
            }
            case CommandKind::BindVertexBuffer: {
                bind_vertex_buffer(encoder, cmd.as<BindVertexBuffer>());
                break;
            }
            case CommandKind::BindIndexBuffer: {
                bind_index_buffer(cmd.as<BindIndexBuffer>());
                break;
            }
            case CommandKind::BindUniformBuffer: {
                bind_uniform_buffer(encoder, cmd.as<BindUniformBuffer>());
                break;
            }

            case CommandKind::DrawArrays: {
                draw_arrays(encoder, cmd.as<DrawArrays>());
                break;
            }
            case CommandKind::DrawIndexed: {
                draw_indexed(encoder, cmd.as<DrawIndexed>());
                break;
            }

            default: UNIMPLEMENTED();
        }
    }

    m_bindings = {};

    encoder->endEncoding();
}

auto MetalCommandExecutor::execute_transfer_pass(
    [[maybe_unused]] TransferPassDescriptor&&       descriptor,
    [[maybe_unused]] const std::span<const Command> cmds
) -> void {
    auto cmd_encoder = m_cmd_buffer->blitCommandEncoder();

    for (const auto& cmd : cmds) {
        switch (cmd.type) {
            case CommandKind::UploadToBuffer: {
                upload_to_buffer(cmd_encoder, cmd.as<UploadToBuffer>());
                break;
            }
            default: UNIMPLEMENTED();
        }
    }

    cmd_encoder->endEncoding();
}

auto MetalCommandExecutor::bind_graphics_pipeline(
    MTL::RenderCommandEncoder*  encoder,
    const BindGraphicsPipeline& bind_graphics_pipeline
) -> void {
    const auto& descriptor = m_state.pipelines.details(bind_graphics_pipeline.pipeline_handle);
    auto* pipeline_state   = m_state.pipelines.fetch(bind_graphics_pipeline.pipeline_handle).get();
    encoder->setRenderPipelineState(pipeline_state);

    m_bindings.pipeline = descriptor;

    // we set format, alpha mode, color and alpha blend during creation of the RenderPipelineState
    // so for now we can skip

    // depth stencil pipeline state
    if (descriptor.depth_stencil.has_value()) {
        // TODO: where do we set depth stencil pixel format? no way to set atm I think...
        auto ds_desc = metal::transfer_ptr(MTL::DepthStencilDescriptor::alloc()->init());
        ds_desc->setDepthCompareFunction(
            metal::compare_function(descriptor.depth_stencil->depth_function)
        );
        ds_desc->setDepthWriteEnabled(descriptor.depth_stencil->depth_write);

        auto ds_state =
            metal::transfer_ptr(m_cmd_buffer->device()->newDepthStencilState(ds_desc.get()));
        encoder->setDepthStencilState(ds_state.get());
    }

    // cull mode
    encoder->setCullMode(metal::cull_mode(descriptor.cull_mode));
    // force always CCW winding
    encoder->setFrontFacingWinding(MTL::WindingCounterClockwise);
}

auto MetalCommandExecutor::bind_vertex_buffer(
    MTL::RenderCommandEncoder* encoder,
    const BindVertexBuffer&    bind_vertex_buffer
) -> void {
    auto buf = m_state.buffers.fetch(bind_vertex_buffer.vertex_buffer);
    encoder->setVertexBuffer(buf.get(), bind_vertex_buffer.offset, bind_vertex_buffer.slot);
}

auto MetalCommandExecutor::bind_index_buffer(const BindIndexBuffer& bind_index_buffer) -> void {
    m_bindings.index_buffer = Bindings::IndexBuf{
        .buf    = m_state.buffers.fetch(bind_index_buffer.index_buffer).get(),
        .format = bind_index_buffer.index_format,
    };
}

auto MetalCommandExecutor::bind_uniform_buffer(
    MTL::RenderCommandEncoder* encoder,
    const BindUniformBuffer&   bind_uniform_buffer
) -> void {
    auto buf = m_state.buffers.fetch(bind_uniform_buffer.uniform_buffer);
    encoder->setVertexBuffer(buf.get(), 0, bind_uniform_buffer.slot);
}

auto MetalCommandExecutor::draw_arrays(
    MTL::RenderCommandEncoder* encoder,
    const DrawArrays&          draw_arrays
) -> void {
    ASSERT(
        m_bindings.pipeline.has_value(), "cannot draw arrays without a bound graphics pipeline."
    );

    encoder->drawPrimitives(
        metal::primitive_type(m_bindings.pipeline->topology), draw_arrays.start, draw_arrays.count
    );
}

auto MetalCommandExecutor::draw_indexed(
    MTL::RenderCommandEncoder* encoder,
    const DrawIndexed&         draw_indexed
) -> void {
    ASSERT(m_bindings.index_buffer.has_value(), "cannot draw indexed without bound index buffer.");

    ASSERT(
        m_bindings.pipeline.has_value(), "cannot draw arrays without a bound graphics pipeline."
    );

    encoder->drawIndexedPrimitives(
        metal::primitive_type(m_bindings.pipeline->topology),
        draw_indexed.index_count,
        metal::index_type(m_bindings.index_buffer->format),
        m_bindings.index_buffer->buf,
        0
    );
}

auto MetalCommandExecutor::upload_to_buffer(
    MTL::BlitCommandEncoder*,
    const UploadToBuffer& upload_to_buffer
) -> void {
    auto        buf     = m_state.buffers.fetch(upload_to_buffer.buffer);
    const auto& details = m_state.buffers.details(upload_to_buffer.buffer);

    ASSERT(
        details.size >= upload_to_buffer.data.size() + upload_to_buffer.offset,
        "destination buffer is too small to upload data onto!"
    );

    ASSERT(details.usage == BufferUsage::Dynamic, "upload to static buffers is not yet supported.");

    std::memcpy(buf->contents(), upload_to_buffer.data.data(), upload_to_buffer.data.size());
}

}; // namespace siren
