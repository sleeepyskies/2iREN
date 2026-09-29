#pragma once

#include "2iREN/graphics/backend/opengl/resource_state.hpp"
#include "2iREN/graphics/commands.hpp"

namespace siren::opengl {

// OpenGL has no concept of a command buffer or encoder. Therefore, to have
// the Gpu work only be done once :wq

class RenderCommandEncoder final : public ::siren::RenderCommandEncoder {
public:
    auto bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `bind_graphics_pipeline` is not implemented.");
    }

    auto bind_vertex_buffer(BufferHandle buffer, Slot slot, Range<usize> range = {})
        -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `bind_vertex_buffer` is not implemented.");
    }

    auto bind_index_buffer(BufferHandle buffer, IndexType type) -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `bind_index_buffer` is not implemented.");
    }

    auto bind_uniform_buffer(BufferHandle buffer, Slot slot, Range<usize> range = {})
        -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `bind_uniform_buffer` is not implemented.");
    }

    auto bind_storage_buffer(BufferHandle buffer, Slot slot, Range<usize> range = {})
        -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `bind_storage_buffer` is not implemented.");
    }

    auto bind_image(ImageHandle image, Slot slot) -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `bind_image` is not implemented.");
    }

    auto bind_sampler(SamplerHandle sampler, Slot slot) -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `bind_sampler` is not implemented.");
    }

    auto draw(u32 count, u32 start = 0, u32 instance_count = 1, u32 instance_start = 0)
        -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `draw` is not implemented.");
    }

    auto draw_indexed(u32 count, u32 start = 0) -> void override {
        // TODO: Implement this pure virtual method.
        static_assert(false, "Method `draw_indexed` is not implemented.");
    }
};

class CommandBuffer final : public ::siren::CommandBuffer {
public:
    CommandBuffer(const ResourceState& state) : m_state(state) { }

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

    auto copy_image_to_buffer(ImageHandle src, BufferHandle dst, usize dst_offset)
        -> void override;

    auto copy_image_to_image(ImageHandle src, ImageHandle dst) -> void override;

private:
    const ResourceState& m_state;
};

} // namespace siren::opengl
