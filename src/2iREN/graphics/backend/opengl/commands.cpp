#include "commands.hpp"

#include <glad/gl.h>

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/backend/opengl/mappings.hpp"
#include "2iREN/graphics/types.hpp"

namespace siren::opengl {

auto RenderCommandEncoder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void { }

auto CommandBuffer::render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& encode)
    -> void {
    auto command_encoder = RenderCommandEncoder{};
    std::invoke(encode, command_encoder);
}

auto CommandBuffer::fill_buffer(BufferHandle buffer, u8 value, Range<usize> range) -> void {
    const auto& descriptor = m_state.buffers.details(buffer);
    ASSERT(descriptor.memory_usage == MemoryUsage::Shared, "cannot clear private buffer");
    const auto glbuffer = m_state.buffers.fetch(buffer);
    glClearNamedBufferSubData(glbuffer, GL_R8UI, range.begin, range.size(), GL_RED_INTEGER, GL_UNSIGNED_BYTE, &value);
}

auto CommandBuffer::write_buffer(BufferHandle dst, usize dst_offset, ByteBufferView data) -> void {
    const auto& descriptor = m_state.buffers.details(dst);
    ASSERT(descriptor.memory_usage == MemoryUsage::Shared, "cannot write to private buffer");
    const auto glbuffer = m_state.buffers.fetch(dst);
    glNamedBufferSubData(glbuffer, dst_offset, data.size(), data.data());
}

auto CommandBuffer::write_image(ImageHandle dst, ByteBufferView data, u32 layer) -> void {
    const auto glimage = m_state.images.fetch(dst);
    const auto& descriptor = m_state.images.details(dst).descriptor;

    switch (descriptor.dimension) {
        case ImageDimension::D1: {
            glTextureSubImage1D(
                glimage,
                0,
                0,
                static_cast<GLsizei>(descriptor.extent.x),
                img_format_to_gl_layout(descriptor.format),
                GL_UNSIGNED_BYTE,
                data.data()
            );
            break;
        }

        case ImageDimension::D2: {
            glTextureSubImage2D(
                glimage,
                0,
                0,
                0,
                static_cast<GLsizei>(descriptor.extent.x),
                static_cast<GLsizei>(descriptor.extent.y),
                img_format_to_gl_layout(descriptor.format),
                GL_UNSIGNED_BYTE,
                data.data()
            );
            break;
        }

        case ImageDimension::D3: {
            glTextureSubImage3D(
                glimage,
                0,
                0,
                0,
                0,
                static_cast<GLsizei>(descriptor.extent.x),
                static_cast<GLsizei>(descriptor.extent.y),
                static_cast<GLsizei>(descriptor.extent.z),
                img_format_to_gl_layout(descriptor.format),
                GL_UNSIGNED_BYTE,
                data.data()
            );
            break;
        }

        case ImageDimension::Cube: {
            glTextureSubImage3D(
                glimage,
                0,
                0,
                0,
                static_cast<i32>(layer),
                static_cast<GLsizei>(descriptor.extent.x),
                static_cast<GLsizei>(descriptor.extent.y),
                1,
                opengl::img_format_to_gl_layout(descriptor.format),
                GL_UNSIGNED_BYTE,
                data.data()
            );
        }
    }
}

auto CommandBuffer::copy_buffer_to_buffer(
    BufferHandle src,
    Range<usize> src_range,
    BufferHandle dst,
    usize dst_offset
) -> void {
    const auto& dst_descriptor = m_state.buffers.details(dst);

    ASSERT(src_range.size() <= dst_descriptor.size - dst_offset, "destination buffer is not large enough to hold requested data");

    const auto srchandle = m_state.buffers.fetch(src);
    const auto dsthandle = m_state.buffers.fetch(dst);

    glCopyNamedBufferSubData(srchandle, dsthandle, src_range.begin, dst_offset, src_range.size());
}

auto CommandBuffer::copy_buffer_to_image(BufferHandle src, usize src_offset, ImageHandle dst) -> void {
    const auto glbuffer = m_state.buffers.fetch(src);
    const auto& descriptor = m_state.images.details(dst).descriptor;
    const auto glimage = m_state.images.fetch(dst);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glbuffer);
    glTextureSubImage2D(
        glimage,
        0,
        0,
        0,
        descriptor.extent.x,
        descriptor.extent.y,
        img_format_to_gl_layout(descriptor.format),
        image_format_channel_type(descriptor.format),
        (void*)src_offset
    );
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

auto CommandBuffer::copy_image_to_buffer(ImageHandle src, BufferHandle dst, usize dst_offset)
    -> void {
    const auto glimage = m_state.images.fetch(src);
    const auto& img_descriptor = m_state.images.details(src).descriptor;

    const auto glbuffer = m_state.buffers.fetch(dst);
    const auto& buf_descriptor = m_state.buffers.details(dst);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, glbuffer);
    
    glGetTextureSubImage(
        glimage,
        0,
        0, 
        0, 
        0,
        img_descriptor.extent.x,
        img_descriptor.extent.y,
        img_descriptor.extent.z,
        img_format_to_gl_layout(img_descriptor.format),
        image_format_channel_type(img_descriptor.format),
        buf_descriptor.size.get() - dst_offset,
        (void*)(dst_offset)
    );
    
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

auto CommandBuffer::copy_image_to_image(ImageHandle src, ImageHandle dst) -> void {
    const auto srchandle = m_state.images.fetch(src);
    const auto dsthandle = m_state.images.fetch(dst);
    const auto& src_desriptor = m_state.images.details(src).descriptor;
    const auto& dst_desriptor = m_state.images.details(dst).descriptor;

    ASSERT(
        src_desriptor.format == dst_desriptor.format,
        "copy between images must share the same format"
    );

    ASSERT(
        src_desriptor.dimension == dst_desriptor.dimension,
        "copy between images must share the same dimensionality"
    );

    glCopyImageSubData(
        srchandle,
        img_to_target_gl(src_desriptor.extent, src_desriptor.dimension),
        0,
        0,
        0,
        0,
        dsthandle,
        img_to_target_gl(dst_desriptor.extent, dst_desriptor.dimension),
        0,
        0,
        0,
        0,
        src_desriptor.extent.x,
        src_desriptor.extent.y,
        src_desriptor.extent.z
    );
}

} // namespace siren
