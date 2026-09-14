#include "command_executor.hpp"

#include <Foundation/NSSharedPtr.hpp>
#include <Metal/MTLCommandBuffer.hpp>
#include <Metal/MTLCommandQueue.hpp>
#include <Metal/Metal.hpp>

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/backend/metal/device.hpp"
#include "2iREN/graphics/backend/metal/mappings.hpp"
#include "2iREN/graphics/backend/metal/util.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/statistics.hpp"

namespace siren {

MetalCommandExecutor::MetalCommandExecutor(
    MetalDeviceState& state,
    NS::SharedPtr<MTL::CommandBuffer> cmd_buffer
) : m_state(state), m_cmd_buffer(cmd_buffer) { }

auto MetalCommandExecutor::execute(RenderPass&& pass) -> void {
    auto& descriptor = pass.descriptor;
    auto& cmds       = pass.commands;

    // setup target
    auto desc = metal::transfer_ptr(MTL::RenderPassDescriptor::alloc()->init());

    for (usize i = 0; i < descriptor.target.colors.size(); i++) {
        auto& attachment     = descriptor.target.colors[i];
        auto* mtl_attachment = desc->colorAttachments()->object(i);
        auto* mtl_texture    = m_state.images.fetch(attachment.image);

        const auto& rgba = attachment.clear_color;

        mtl_attachment->setTexture(mtl_texture);
        mtl_attachment->setLoadAction(metal::load_action(attachment.begin_operation));
        mtl_attachment->setClearColor(MTL::ClearColor::Make(rgba.r, rgba.g, rgba.b, rgba.a));
        mtl_attachment->setStoreAction(MTL::StoreActionStore); // TODO: idk bro
    }

    if (descriptor.target.depth_stencil) {
        // setup depth
        {
            auto& attachment     = *descriptor.target.depth_stencil;
            auto* mtl_attachment = desc->depthAttachment();
            auto* mtl_texture    = m_state.images.fetch(attachment.image);

            mtl_attachment->setTexture(mtl_texture);
            mtl_attachment->setLoadAction(metal::load_action(attachment.begin_operation));
            mtl_attachment->setClearDepth(attachment.clear_depth);
            mtl_attachment->setStoreAction(MTL::StoreActionStore); // TODO: idk bro
        }

        // setup stencil
        {
            auto& attachment     = *descriptor.target.depth_stencil;
            auto* mtl_attachment = desc->stencilAttachment();
            auto* mtl_texture    = m_state.images.fetch(attachment.image);

            mtl_attachment->setTexture(mtl_texture);
            mtl_attachment->setLoadAction(metal::load_action(attachment.begin_operation));
            mtl_attachment->setClearStencil(attachment.clear_stencil);
            mtl_attachment->setStoreAction(MTL::StoreActionStore); // TODO: idk bro
        }
    }

    m_cmd_encoder = metal::retain_ptr(m_cmd_buffer->renderCommandEncoder(desc.get()));

    for (const auto& cmd : cmds) {
        switch (cmd.type) {
            case RenderCommandType::BindGraphicsPipeline: {
                bind_graphics_pipeline(cmd.as<BindGraphicsPipeline>());
                break;
            }
            case RenderCommandType::BindVertexBuffer: {
                bind_vertex_buffer(cmd.as<BindVertexBuffer>());
                break;
            }
            case RenderCommandType::DrawArrays: {
                draw_arrays(cmd.as<DrawArrays>());
                break;
            }
            default: UNIMPLEMENTED();
        }
    }

    m_cmd_encoder->endEncoding();
}

auto MetalCommandExecutor::bind_graphics_pipeline(
    const BindGraphicsPipeline& bind_graphics_pipeline
) -> void {
    auto* pipeline_state = m_state.pipelines.fetch(bind_graphics_pipeline.pipeline_handle);
    m_cmd_encoder->setRenderPipelineState(pipeline_state);
}

auto MetalCommandExecutor::bind_vertex_buffer(const BindVertexBuffer& bind_vertex_buffer) -> void {
    auto buf = m_state.buffers.fetch(bind_vertex_buffer.vertex_buffer);
    m_cmd_encoder->setVertexBuffer(buf.get(), bind_vertex_buffer.offset, bind_vertex_buffer.slot);
}

auto MetalCommandExecutor::draw_arrays(const DrawArrays& draw_arrays) -> void {
    m_cmd_encoder->drawPrimitives(
        metal::primitive_type(draw_arrays.primitive_topology), draw_arrays.start, draw_arrays.count
    );
}

auto MetalCommandExecutor::statistics() const -> const Statistics& {
    return m_statistics;
}

}; // namespace siren
