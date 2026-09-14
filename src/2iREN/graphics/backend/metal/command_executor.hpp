#pragma once

#include "2iREN/graphics/backend/metal/fwd.hpp"
#include "2iREN/graphics/command_executor.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/statistics.hpp"

namespace siren {

struct MetalDeviceState;

class MetalCommandExecutor final : public CommandExecutor {
public:
    MetalCommandExecutor(
        MetalDeviceState& state,
        MTL::CommandBuffer* cmd_buffer
    );

    auto execute(RenderPass&& pass) -> void override;

    [[nodiscard]]
    auto statistics() const -> const Statistics& override;

private:
    auto bind_graphics_pipeline(const BindGraphicsPipeline&) -> void;
    auto bind_vertex_buffer(const BindVertexBuffer&) -> void;
    auto draw_arrays(const DrawArrays&) -> void;

private:
    MetalDeviceState& m_state;
    Statistics m_statistics;
    MTL::CommandBuffer* m_cmd_buffer;
    MTL::RenderCommandEncoder* m_cmd_encoder;
};

} // namespace siren
