#pragma once

#include <Metal/MTLCommandBuffer.hpp>
#include <optional>

#include "2iREN/graphics/backend/metal/resource_state.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"

namespace siren::metal {

class RenderCommandEncoder final : public ::siren::RenderCommandEncoder {
public:
    RenderCommandEncoder(
        const ResourceState& state,
        NS::SharedPtr<MTL::RenderCommandEncoder> encoder
    ) : m_state(state), m_encoder(encoder) { }

    auto bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void override;
    auto bind_vertex_buffer(BufferHandle buffer, u32 slot, u32 offset) -> void override;
    auto bind_index_buffer(BufferHandle buffer, IndexFormat index_format) -> void override;
    auto bind_uniform_buffer(BufferHandle buffer, u32 slot, u32 offset) -> void override;
    auto bind_storage_buffer(BufferHandle buffer, u32 slot) -> void override;
    auto bind_image(ImageHandle image, u32 slot) -> void override;
    auto draw_arrays(u32 start, u32 count) -> void override;
    auto draw_indexed(u32 index_count, u32 first_index) -> void override;

private:
    struct BindIdxBuf {
        BufferHandle buffer;
        IndexFormat format;
    };

    struct Bindings {
        std::optional<GraphicsPipelineDescriptor> pipeline = std::nullopt;
        std::optional<BindIdxBuf> index                    = std::nullopt;
    } m_bindings = {};
    const ResourceState& m_state;
    NS::SharedPtr<MTL::RenderCommandEncoder> m_encoder;
};

class CommandBuffer final : public ::siren::CommandBuffer {
public:
    CommandBuffer(MTL::CommandBuffer* cmdbuffer, const ResourceState& state) :
        m_state(state), m_cmdbuffer(cmdbuffer) { }

    auto render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& function)
        -> void override;

    auto write_buffer(BufferHandle dest, u32 dest_offset, const ByteBufferView data)
        -> void override;

    auto mtl_command_buffer() -> MTL::CommandBuffer* {
        return m_cmdbuffer;
    }

private:
    const ResourceState& m_state;
    MTL::CommandBuffer* m_cmdbuffer;
};

} // namespace siren::metal
