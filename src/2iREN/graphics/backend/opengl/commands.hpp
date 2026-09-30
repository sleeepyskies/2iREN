#pragma once

#include "2iREN/graphics/backend/opengl/framebuffer_cache.hpp"
#include "2iREN/graphics/backend/opengl/resource_state.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/fwd.hpp"

namespace siren::opengl {

// OpenGL has no concept of a command buffer or encoder. Therefore, to have
// the Gpu work only be done once :wq

class RenderCommandEncoder final : public ::siren::RenderCommandEncoder {
public:
    RenderCommandEncoder(const ResourceState& state, const bool has_depth_attachment) :
        m_state(state), m_has_depth_attachment(has_depth_attachment) { }
    ~RenderCommandEncoder();

    auto bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void override;

    auto bind_vertex_buffer(BufferHandle buffer, Slot slot, const u32 offset) -> void override;

    auto bind_index_buffer(BufferHandle buffer, IndexType type) -> void override;

    auto bind_uniform_buffer(BufferHandle buffer, Slot slot, u32 offset = 0) -> void override;

    auto bind_storage_buffer(BufferHandle buffer, Slot slot, u32 offset = 0) -> void override;

    auto bind_sampled_image(ImageHandle image, SamplerHandle sampler, Slot slot) -> void override;

    auto bind_storage_image(ImageHandle image, Slot slot) -> void override;

    auto draw(u32 count, u32 start = 0, u32 instance_count = 1, u32 instance_start = 0)
        -> void override;

    auto draw_indexed(u32 count, u32 start = 0) -> void override;

private:
    auto reset_pipeline_state() const -> void;

    struct IndexBinding {
        BufferHandle buffer;
        IndexType type;
    };

    mutable struct TrackedState {
        std::optional<GraphicsPipelineHandle> bound_pipeline = std::nullopt;
        std::optional<GLuint> bound_vao                      = std::nullopt;
        std::optional<IndexBinding> bound_index              = std::nullopt;

        auto reset() -> void {
            bound_pipeline.reset();
            bound_vao.reset();
            bound_index.reset();
        }
    } m_tracked;

    const ResourceState& m_state;
    bool m_has_depth_attachment;
};

class CommandBuffer final : public ::siren::CommandBuffer {
public:
    CommandBuffer(const ResourceState& state, FramebufferCache& framebuffer_cache) :
        m_state(state), m_framebuffer_cache(framebuffer_cache) { }

    auto render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& encode)
        -> void override;

    auto fill_buffer(BufferHandle buffer, u8 value, Range<usize> range = {}) -> void override;

    auto write_buffer(BufferHandle dst, usize dst_offset, ByteBufferView data) -> void override;

    auto WriteBuffer();

    auto write_image(ImageHandle dst, ByteBufferView data, u32 layer = 0) -> void override;

    auto copy_buffer_to_buffer(
        BufferHandle src,
        Range<usize> src_range,
        BufferHandle dst,
        usize dst_offset
    ) -> void override;

    auto copy_buffer_to_image(BufferHandle src, usize src_offset, ImageHandle dst) -> void override;

    auto copy_image_to_buffer(ImageHandle src, BufferHandle dst, usize dst_offset) -> void override;

    auto copy_image_to_image(ImageHandle src, ImageHandle dst) -> void override;

private:
    struct RenderPassState {
        GLuint framebuffer        = 0;
        bool default_framebuffer  = false;
        bool has_depth_attachment = false;
    };

    [[nodiscard]]
    auto begin_render_pass(const RenderPassDescriptor& descriptor) const -> RenderPassState;

    auto end_render_pass(const RenderPassDescriptor& descriptor, const RenderPassState& state) const
        -> void;

    const ResourceState& m_state;
    FramebufferCache& m_framebuffer_cache;
};

} // namespace siren::opengl
