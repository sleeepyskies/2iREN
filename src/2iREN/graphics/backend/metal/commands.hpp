#pragma once

#include <Metal/MTLRenderCommandEncoder.hpp>
#include <optional>

#include <Metal/MTLCommandBuffer.hpp>

#include "2iREN/graphics/backend/metal/resource_state.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"

namespace siren::metal {

/// @brief Metal implementation of the siren::RenderCommandEncoder.
class RenderCommandEncoder final : public ::siren::RenderCommandEncoder {
public:
    RenderCommandEncoder(
        const ResourceState& state,
        NS::SharedPtr<MTL::RenderCommandEncoder> encoder
    ) : m_state(state), m_encoder(encoder) { }

    auto bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void override;
    auto bind_vertex_buffer(BufferHandle buffer, Slot slot, Range<usize> range) -> void override;
    auto bind_index_buffer(BufferHandle buffer, IndexType type) -> void override;
    auto bind_uniform_buffer(BufferHandle buffer, Slot slot, Range<usize> range) -> void override;
    auto bind_storage_buffer(BufferHandle buffer, Slot slot, Range<usize> range) -> void override;
    auto bind_image(ImageHandle image, Slot slot) -> void override;
    auto bind_sampler(SamplerHandle sampler, Slot slot) -> void override;
    auto draw(u32 count, u32 start, u32 instance_count, u32 instance_start) -> void override;
    auto draw_indexed(u32 count, u32 start) -> void override;

private:
    struct BindIdxBuf {
        BufferHandle buffer;
        IndexType format;
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

    auto fill_buffer(BufferHandle buffer, u8 value, Range<usize> range) -> void override;

    auto write_buffer(BufferHandle buffer, usize buffer_offset, ByteBufferView data)
        -> void override;
    auto write_image(ImageHandle image, ByteBufferView data, u32 layer) -> void override;

    auto copy_buffer_to_buffer(
        BufferHandle src,
        Range<usize> src_range,
        BufferHandle dest,
        usize dest_offset
    ) -> void override;

    auto copy_buffer_to_image(BufferHandle src, usize src_offset, ImageHandle dst) -> void override;
    auto copy_image_to_buffer(ImageHandle src, BufferHandle dst, usize dst_offset) -> void override;
    auto copy_image_to_image(ImageHandle src, ImageHandle dst) -> void override;

    /// @HACK: custom imgui backend would be best
    auto metal_commandbuffer() -> MTL::CommandBuffer* {
        return m_cmdbuffer;
    }

private:
    const ResourceState& m_state;
    MTL::CommandBuffer* m_cmdbuffer;
};

} // namespace siren::metal
