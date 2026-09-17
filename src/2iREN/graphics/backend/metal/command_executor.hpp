#pragma once

#include <Metal/MTLRenderCommandEncoder.hpp>
#include <optional>
#include <span>

#include <Metal/MTLCommandBuffer.hpp>

#include "2iREN/graphics/backend/metal/fwd.hpp"
#include "2iREN/graphics/command_executor.hpp"
#include "2iREN/graphics/commands.hpp"

namespace siren {

struct MetalDeviceState;

class MetalCommandExecutor final : public CommandExecutor {
public:
    MetalCommandExecutor(MetalDeviceState& state, NS::SharedPtr<MTL::CommandBuffer> cmd_buffer);

    auto execute(CommandList&& cmds) -> void override;

private:
    auto execute_render_pass(RenderPassDescriptor&& descriptor, const std::span<const Command> cmds)
        -> void;
    auto execute_transfer_pass(
        TransferPassDescriptor&&       descriptor,
        const std::span<const Command> cmds
    ) -> void;

    // RENDER COMMANDS
    auto bind_graphics_pipeline(MTL::RenderCommandEncoder*, const BindGraphicsPipeline&) -> void;
    auto bind_vertex_buffer(MTL::RenderCommandEncoder*, const BindVertexBuffer&) -> void;
    auto bind_index_buffer(const BindIndexBuffer&) -> void;
    auto bind_uniform_buffer(MTL::RenderCommandEncoder*, const BindUniformBuffer&) -> void;

    auto draw_arrays(MTL::RenderCommandEncoder*, const DrawArrays&) -> void;
    auto draw_indexed(MTL::RenderCommandEncoder*, const DrawIndexed&) -> void;

    // TRANSFER COMMANDS
    auto upload_to_buffer(MTL::BlitCommandEncoder*, const UploadToBuffer&) -> void;

private:
    MetalDeviceState&                 m_state;
    NS::SharedPtr<MTL::CommandBuffer> m_cmd_buffer = nullptr;

    struct Bindings {
        std::optional<BindIndexBuffer> index_buffer = std::nullopt;
    } m_bindings = {};
};

} // namespace siren
