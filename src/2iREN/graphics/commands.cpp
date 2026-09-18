#include "commands.hpp"

#include "2iREN/graphics/device.hpp"

namespace siren {

RenderCommandRecorder::RenderCommandRecorder(const Device* device) {
    // TODO: query device for limits, then create ararys/vectos instead of maps
    // for the actuve bindsings
    [[maybe_unused]] const auto& limits = device->limits();

    // TODO: what size is best to allocate up front?
    m_commands.reserve(32);
}

auto RenderCommandRecorder::bind_graphics_pipeline(const GraphicsPipelineHandle pipeline) noexcept
    -> void {
    if (pipeline == m_active_pipeline) {
        return;
    }
    m_active_pipeline = pipeline;

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_graphics_pipeline = {.pipeline_handle = pipeline},
                },
            .type = CommandKind::BindGraphicsPipeline
        }
    );
}

auto RenderCommandRecorder::bind_vertex_buffer(
    const BufferHandle buffer,
    const u32          slot,
    const u32          offset
) noexcept -> void {
    const auto& it = m_active_vertex_buffers.find(slot);
    if (it != m_active_vertex_buffers.end() && it->second == buffer) {
        return;
    }

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_vertex_buffer =
                        {
                            .vertex_buffer = buffer,
                            .slot          = slot,
                            .offset        = offset,
                        },
                },
            .type = CommandKind::BindVertexBuffer,
        }
    );

    m_active_vertex_buffers[slot] = buffer;
}

auto RenderCommandRecorder::bind_index_buffer(
    const BufferHandle buffer,
    const IndexFormat  index_format
) noexcept -> void {
    if (m_active_index_buffer.has_value()) {
        const auto& active = m_active_index_buffer.value();
        if (active.index_buffer == buffer && active.index_format == index_format) {
            return;
        }
    }

    const BindIndexBuffer cmd{.index_buffer = buffer, .index_format = index_format};

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_index_buffer = cmd,
                },
            .type = CommandKind::BindIndexBuffer
        }
    );

    m_active_index_buffer = cmd;
}

auto RenderCommandRecorder::bind_uniform_buffer(const BufferHandle buffer, const u32 slot) noexcept
    -> void {
    const auto& it = m_active_uniform_buffers.find(slot);
    if (it != m_active_vertex_buffers.end() && it->second == buffer) {
        return;
    }

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_uniform_buffer =
                        {
                            .uniform_buffer = buffer,
                            .slot           = slot,
                        },
                },
            .type = CommandKind::BindUniformBuffer,
        }
    );

    m_active_uniform_buffers[slot] = buffer;
}

auto RenderCommandRecorder::bind_uniform_buffer_range(
    const BufferHandle buffer,
    const u32          slot,
    const usize        offset,
    const usize        size
) noexcept -> void {
    // TODO: check we dont bind same buffer and range twice in a row
    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_uniform_buffer_range =
                        {
                            .uniform_buffer = buffer,
                            .slot           = slot,
                            .offset         = offset,
                            .size           = size,
                        },
                },
            .type = CommandKind::BindUniformBufferRange,
        }
    );

    m_active_uniform_buffers[slot] = buffer;
}

auto RenderCommandRecorder::bind_shader_storage_buffer(
    const BufferHandle buffer,
    const u32          slot
) noexcept -> void {
    const auto& it = m_active_shader_storage_buffers.find(slot);
    if (it != m_active_shader_storage_buffers.end() && it->second == buffer) {
        return;
    }

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_shader_storage_buffer =
                        {
                            .shader_storage_buffer = buffer,
                            .slot                  = slot,
                        },
                },
            .type = CommandKind::BindShaderStorageBuffer,
        }
    );

    m_active_shader_storage_buffers[slot] = buffer;
}

auto RenderCommandRecorder::bind_sampled_image(
    const ImageHandle   image,
    const SamplerHandle sampler,
    const u32           slot
) noexcept -> void {
    const auto& it = m_sampled_images.find(slot);
    if (it != m_sampled_images.end() && it->second == image) {
        return;
    }

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_sampled_image =
                        {
                            .image   = image,
                            .sampler = sampler,
                            .slot    = slot,
                        },
                },
            .type = CommandKind::BindSampledImage,
        }
    );

    m_sampled_images[slot] = image;
}

auto RenderCommandRecorder::bind_storage_image(
    const ImageHandle image,
    const AccessKind  access,
    const u32         slot
) noexcept -> void {
    const auto& it = m_storage_images.find(slot);
    if (it != m_storage_images.end() && it->second == image) {
        return;
    }

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .bind_storage_image =
                        {
                            .image  = image,
                            .slot   = slot,
                            .access = access,
                        },
                },
            .type = CommandKind::BindStorageImage,
        }
    );

    m_storage_images[slot] = image;
}

auto RenderCommandRecorder::begin_query(const QueryHandle handle) noexcept -> void {
    m_commands.emplace_back(
        Command{.command = {.begin_query = {.query = handle}}, .type = CommandKind::BeginQuery}
    );
}

auto RenderCommandRecorder::end_query(const QueryHandle handle) noexcept -> void {
    m_commands.emplace_back(
        Command{
            .command = {.end_query = {.query = handle}},
            .type    = CommandKind::EndQuery,
        }
    );
}

auto RenderCommandRecorder::draw_arrays(const u32 start, const u32 count) noexcept -> void {
    ASSERT(
        m_active_pipeline.is_valid(),
        "there is no pipeline bound, cannot call "
        "RenderCommandRecorder::draw_arrays."
    );

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .draw_arrays =
                        {
                            .start = start,
                            .count = count,
                        },
                },
            .type = CommandKind::DrawArrays
        }
    );
}

auto RenderCommandRecorder::draw_indexed(const u32 index_count, const u32 first_index) noexcept
    -> void {
    ASSERT(
        m_active_pipeline.is_valid(),
        "there is no pipeline bound, cannot call "
        "RenderCommandRecorder::draw_indexed."
    );
    ASSERT(
        m_active_index_buffer.has_value() && m_active_index_buffer.value().index_buffer.is_valid(),
        "there is no index buffer bound, cannot call "
        "RenderCommandRecorder::draw_indexed."
    );
    ASSERT(
        m_active_vertex_buffers.size() > 0,
        "there are no vertex buffers bound, cannot call "
        "RenderCommandRecorder::draw_indexed."
    );

    m_commands.emplace_back(
        Command{
            .command =
                {
                    .draw_indexed =
                        {
                            .first_index = first_index,
                            .index_count = index_count,
                        },
                },
            .type = CommandKind::DrawIndexed,
        }
    );
}

auto RenderCommandRecorder::finish() && -> Commands {
    // TODO: could we perform some more optimizations here on the commands?
    // maybe this can be toggled or something with a compile flag,
    // but reordering commands coul be useful here, or maybe just costly??
    return std::move(m_commands);
}

TransferCommandRecorder::TransferCommandRecorder([[maybe_unused]] const Device* device) {
    m_commands.reserve(8);
}

auto TransferCommandRecorder::upload_to_buffer(
    const BufferHandle   buffer,
    const ByteBufferView data,
    const u32            offset
) -> void {
    m_commands.emplace_back(
        Command{
            .command =
                {
                    .upload_to_buffer =
                        {
                            .buffer = buffer,
                            .data   = data,
                            .offset = offset,
                        },
                },
            .type = CommandKind::UploadToBuffer,
        }
    );
}

auto TransferCommandRecorder::finish() && -> Commands {
    // TODO: could we perform some more optimizations here on the commands?
    // maybe this can be toggled or something with a compile flag,
    // but reordering commands coul be useful here, or maybe just costly??
    return std::move(m_commands);
}

CommandRecorder::CommandRecorder(const Device* device) : m_device(device) { }

auto CommandRecorder::render_pass(
    const RenderPassDescriptor& descriptor,
    RenderPassFunction&&        function
) -> void {
    auto recorder = RenderCommandRecorder{m_device};

    std::invoke(function, recorder);

    const auto start = m_commands.size();

    m_commands.append_range(std::move(recorder).finish());

    const auto end = m_commands.size();

    m_passes.emplace_back(descriptor, Range<usize>{start, end});
}

auto CommandRecorder::transfer_pass(
    const TransferPassDescriptor& descriptor,
    TransferPassFunction&&        function
) -> void {
    auto recorder = TransferCommandRecorder{m_device};

    std::invoke(function, recorder);

    const auto start = m_commands.size();

    m_commands.append_range(std::move(recorder).finish());

    const auto end = m_commands.size();

    m_passes.emplace_back(descriptor, Range<usize>{start, end});
}

auto CommandRecorder::finish() && -> CommandList {
    return CommandList{
        .passes   = std::move(m_passes),
        .commands = std::move(m_commands),
    };
}

} // namespace siren
