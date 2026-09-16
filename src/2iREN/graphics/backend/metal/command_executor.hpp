#pragma once

#include <Metal/Metal.hpp>
#include <span>

#include "2iREN/graphics/command_executor.hpp"

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

    auto bind_graphics_pipeline(const BindGraphicsPipeline&) -> void;
    auto bind_vertex_buffer(const BindVertexBuffer&) -> void;
    auto draw_arrays(const DrawArrays&) -> void;

private:
    MetalDeviceState&                        m_state;
    NS::SharedPtr<MTL::CommandBuffer>        m_cmd_buffer  = nullptr;
    NS::SharedPtr<MTL::RenderCommandEncoder> m_cmd_encoder = nullptr;
};

} // namespace siren
