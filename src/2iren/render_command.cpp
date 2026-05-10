#include "render_command.hpp"


namespace siren {

// ============================================================================
// == MARK: RenderPassRecorder
// ============================================================================

RenderPassRecorder::RenderPassRecorder(const RenderPassDescriptor& descriptor) : m_descriptor(descriptor) {
    // todo: no clue how much is good to reserve here
    m_commands.reserve(1024);
}

auto RenderPassRecorder::bind_graphics_pipeline(const GraphicsPipelineHandle pipeline_handle) noexcept -> void {
    if (pipeline_handle == m_active_pipeline) { return; }

    m_commands.emplace_back(
        RenderCommand{
            .command = {
                .bind_graphics_pipeline = {
                    .pipeline_handle = pipeline_handle
                }
            },
            .type = RenderCommandType::BindGraphicsPipeline
        }
    );

    m_active_pipeline = pipeline_handle;
}

auto RenderPassRecorder::set_viewport(const u32 x, const u32 y, const u32 width, const u32 height) noexcept -> void {
    m_commands.emplace_back(
        RenderCommand{
            .command = {
                .set_viewport = {
                    .x = x,
                    .y = y,
                    .width = width,
                    .height = height,
                }
            },
            .type = RenderCommandType::SetViewport
        }
    );
}

auto RenderPassRecorder::bind_vertex_buffer(
    const BufferHandle vertex_buffer,
    const u32 slot,
    const u32 offset
) noexcept -> void {
    const auto& it = m_active_vertex_buffers.find(slot);
    if (it != m_active_vertex_buffers.end() && it->second == vertex_buffer) { return; }

    m_commands.emplace_back(
        RenderCommand{
            .command = {
                .bind_vertex_buffer = {
                    .vertex_buffer = vertex_buffer,
                    .slot = slot,
                    .offset = offset,
                },
            },
            .type = RenderCommandType::BindVertexBuffer,
        }
    );

    m_active_vertex_buffers[slot] = vertex_buffer;
}

auto RenderPassRecorder::bind_index_buffer(
    const BufferHandle index_buffer,
    const IndexFormat index_format
) noexcept -> void {
    if (m_active_index_buffer.has_value()) {
        const auto& active = m_active_index_buffer.value();
        if (active.index_buffer == index_buffer && active.index_format == index_format) { return; }
    }

    const BindIndexBuffer cmd{ .index_buffer = index_buffer, .index_format = index_format };

    m_commands.emplace_back(
        RenderCommand{
            .command = {
                .bind_index_buffer = cmd,
            },
            .type = RenderCommandType::BindIndexBuffer
        }
    );

    m_active_index_buffer = cmd;
}

auto RenderPassRecorder::bind_uniform_buffer(const BufferHandle uniform_buffer, const u32 slot) noexcept -> void {
    const auto& it = m_active_uniform_buffers.find(slot);
    if (it != m_active_vertex_buffers.end() && it->second == uniform_buffer) { return; }

    m_commands.emplace_back(
        RenderCommand{
            .command = {
                .bind_uniform_buffer = {
                    .uniform_buffer = uniform_buffer,
                    .slot = slot,
                },
            },
            .type = RenderCommandType::BindUniformBuffer,
        }
    );

    m_active_uniform_buffers[slot] = uniform_buffer;
}

auto RenderPassRecorder::draw_arrays(const u32 start, const u32 count) noexcept -> void {
    ASSERT(
        m_active_pipeline.is_valid(),
        "There is no pipeline bound, cannot call RenderPassRecorder::draw_arrays."
    );

    m_commands.emplace_back(
        RenderCommand{
            .command = {
                .draw_arrays = {
                    .start = start,
                    .count = count
                }
            },
            .type = RenderCommandType::DrawArrays
        }
    );
}

auto RenderPassRecorder::draw_indexed(const u32 index_count, const u32 first_index) noexcept -> void {
    ASSERT(
        m_active_pipeline.is_valid(),
        "There is no pipeline bound, cannot call RenderPassRecorder::draw_indexed."
    );
    ASSERT(
        m_active_index_buffer.has_value() && m_active_index_buffer.value().index_buffer.is_valid(),
        "There is no index buffer bound, cannot call RenderPassRecorder::draw_indexed."
    );
    ASSERT(
        m_active_vertex_buffers.size() > 0,
        "There are no vertex buffers bound, cannot call RenderPassRecorder::draw_indexed."
    );

    m_commands.emplace_back(
        RenderCommand{
            .command = {
                .draw_indexed = {
                    .first_index = first_index,
                    .index_count = index_count,
                }
            },
            .type = RenderCommandType::DrawIndexed,
        }
    );
}

auto RenderPassRecorder::finish() -> std::pair<std::vector<RenderCommand>&&, RenderPassDescriptor> {
    return std::pair{ std::move(m_commands), m_descriptor };
}

// ============================================================================
// == MARK: RenderCommandRecorder
// ============================================================================

auto RenderCommandRecorder::begin_render_pass(
    const RenderPassDescriptor& descriptor
) const noexcept -> RenderPassRecorder {
    return RenderPassRecorder{ std::move(descriptor) };
}

auto RenderCommandRecorder::consume_render_pass(
    std::pair<std::vector<RenderCommand>&&, RenderPassDescriptor> commands
) noexcept -> void {
    m_render_passes.emplace_back(
        RenderPass{
            .descriptor = commands.second,
            .start = m_commands.size(),
            .count = commands.first.size(),
        }
    );

    m_commands.insert(
        m_commands.end(),
        std::make_move_iterator(commands.first.begin()),
        std::make_move_iterator(commands.first.end())
    );
}

auto RenderCommandRecorder::finish() noexcept -> RenderCommandBuffer {
    return RenderCommandBuffer{ .commands = std::move(m_commands), .render_passes = std::move(m_render_passes) };
}

} // namespace siren
