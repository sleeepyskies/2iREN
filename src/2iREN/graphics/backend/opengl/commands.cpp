#include "commands.hpp"

#include <glad/gl.h>

#include "2iREN/core/assert.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/backend/opengl/mappings.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/types.hpp"

namespace siren::opengl {

// == RenderCommandEncoder ==

RenderCommandEncoder::~RenderCommandEncoder() {
    reset_pipeline_state();
}

auto RenderCommandEncoder::reset_pipeline_state() const -> void {
    // reset to default opengl state
    // TODO: do we need to reset the image slot state as well????
    glUseProgram(0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    m_tracked.reset();
}

auto RenderCommandEncoder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void {
    reset_pipeline_state();

    const auto vao         = m_state.pipelines.fetch(pipeline);
    const auto& descriptor = m_state.pipelines.details(pipeline);
    const auto glshader    = m_state.shaders.fetch(descriptor.shader);

    m_tracked.bound_pipeline = pipeline;
    m_tracked.bound_vao      = vao;

    // bind the shader and vertex array == vertex layout
    glUseProgram(glshader);
    glBindVertexArray(vao);

    // set render state
    for (usize i = 0; i < descriptor.colors.size(); i++) {
        const auto& color = descriptor.colors[i];
        switch (color.alpha_mode) {
            case AlphaMode::Opaque: {
                glDisablei(GL_BLEND, i);
                break;
            }
            case AlphaMode::Blend: {
                glEnablei(GL_BLEND, i);
                break;
            }
        }

        if (color.alpha_mode == AlphaMode::Blend) {
            glBlendFuncSeparatei(
                static_cast<GLuint>(i),
                blend_factor_to_gl(color.color_blend.source_factor),
                blend_factor_to_gl(color.color_blend.dest_factor),
                blend_factor_to_gl(color.alpha_blend.source_factor),
                blend_factor_to_gl(color.alpha_blend.dest_factor)
            );
            glBlendEquationSeparatei(
                static_cast<GLuint>(i),
                blend_function_to_gl(color.color_blend.function),
                blend_function_to_gl(color.alpha_blend.function)
            );
        }
    }

    // NOTE: opengl uses CCW winding by default
    switch (descriptor.cull_mode) {
        case CullMode::None: {
            glDisable(GL_CULL_FACE);
            break;
        }
        case CullMode::Front: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            break;
        }
        case CullMode::Back: {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            break;
        }
    }

    // configure depth buffer update
    if (const auto& ds = descriptor.depth_stencil) {
        glEnable(GL_DEPTH_TEST);
        ds->depth_write ? glDepthMask(GL_TRUE) : glDepthMask(GL_FALSE);
        glDepthFunc(compare_function_to_gl(ds->compare_function));
    } else {
        glDepthMask(GL_FALSE);    // disable depth write
        glDisable(GL_DEPTH_TEST); // disable depth test
    }
}

auto RenderCommandEncoder::bind_vertex_buffer(
    BufferHandle buffer,
    [[maybe_unused]] Slot slot,
    const u32 offset
) -> void {
    const auto& descriptor = m_state.buffers.details(buffer);

    ASSERT(
        descriptor.flags.test(BufferFlag::Vertex),
        "buffer must have Vertex flag to be bound as a vertex buffer"
    );
    ASSERT(
        m_tracked.bound_pipeline and m_tracked.bound_vao,
        "cannot bind opengl vertex buffer without a bound pipeline"
    );

    const auto vbo                   = m_state.buffers.fetch(buffer);
    const auto& pipeline_descriptor = m_state.pipelines.details(*m_tracked.bound_pipeline);

    // HACK:
    // pipeline assumes the vbo is always bound to slot 0, so we ignore user here for now
    glVertexArrayVertexBuffer(
        *m_tracked.bound_vao,
        0,
        vbo,
        static_cast<GLintptr>(offset),
        static_cast<GLsizei>(pipeline_descriptor.layout.stride)
    );
}

auto RenderCommandEncoder::bind_index_buffer(const BufferHandle buffer, const IndexType type) -> void {
    const auto ibo         = m_state.buffers.fetch(buffer);
    const auto& descriptor = m_state.buffers.details(buffer);

    ASSERT(
        m_tracked.bound_pipeline and m_tracked.bound_vao,
        "cannot bind opengl index buffer without a bound graphics pipeline"
    );
    ASSERT(
        descriptor.flags.test(BufferFlag::Index),
        "buffer must have flag Index to bind as an index buffer"
    );

    m_tracked.bound_index = IndexBinding{buffer, type};
    glVertexArrayElementBuffer(*m_tracked.bound_vao, ibo);
}

auto RenderCommandEncoder::bind_uniform_buffer(
    const BufferHandle buffer,
    const Slot slot,
    const u32 offset
) -> void {
    const auto ubo         = m_state.buffers.fetch(buffer);
    const auto& descriptor = m_state.buffers.details(buffer);
    const auto size        = descriptor.size.get();

    ASSERT(
        descriptor.flags.test(BufferFlag::Uniform),
        "buffer must have flag Uniform to bind as a uniform buffer"
    );
    ASSERT(offset < size, "uniform buffer binding offset is outside the buffer");

    glBindBufferRange(
        GL_UNIFORM_BUFFER,
        slot.value,
        ubo,
        static_cast<GLintptr>(offset),
        static_cast<GLsizeiptr>(size - offset)
    );
}

auto RenderCommandEncoder::bind_storage_buffer(
    const BufferHandle buffer,
    const Slot slot,
    const u32 offset
) -> void {
    const auto ssbo        = m_state.buffers.fetch(buffer);
    const auto& descriptor = m_state.buffers.details(buffer);
    const auto size        = descriptor.size.get();

    ASSERT(
        descriptor.flags.test(BufferFlag::Storage),
        "buffer must have flag Storage to bind as a storage buffer"
    );
    ASSERT(offset < size, "storage buffer binding offset is outside the buffer");

    glBindBufferRange(
        GL_SHADER_STORAGE_BUFFER,
        slot.value,
        ssbo,
        static_cast<GLintptr>(offset),
        static_cast<GLsizeiptr>(size - offset)
    );
}

auto RenderCommandEncoder::bind_sampled_image(
    const ImageHandle image,
    const SamplerHandle sampler,
    const Slot slot
) -> void {
    const auto texture      = m_state.images.fetch(image);
    const auto glsampler    = m_state.samplers.fetch(sampler);
    const auto& image_state = m_state.images.details(image);

    ASSERT(
        !image_state.default_framebuffer,
        "the OpenGL default framebuffer cannot be bound as a sampled image"
    );

    glBindTextureUnit(slot.value, texture);
    glBindSampler(slot.value, glsampler);
}

auto RenderCommandEncoder::bind_storage_image(
    const ImageHandle image,
    const Slot slot
) -> void {
    const auto texture      = m_state.images.fetch(image);
    const auto& image_state = m_state.images.details(image);
    const auto& descriptor  = image_state.descriptor;

    ASSERT(
        !image_state.default_framebuffer,
        "the OpenGL default framebuffer cannot be bound as a storage image"
    );
    ASSERT(
        descriptor.flags.any(ImageFlag::ShaderRead, ImageFlag::ShaderWrite),
        "a storage image requires ImageFlag::ShaderRead or ImageFlag::ShaderWrite"
    );

    const auto readable = descriptor.flags.test(ImageFlag::ShaderRead);
    const auto writable = descriptor.flags.test(ImageFlag::ShaderWrite);
    const auto access   = readable and writable ? GL_READ_WRITE
        : readable                             ? GL_READ_ONLY
                                               : GL_WRITE_ONLY;

    const auto layered = descriptor.dimension == ImageDimension::D3
        or descriptor.dimension == ImageDimension::Cube
        or descriptor.extent.z > 1;

    glBindImageTexture(
        slot.value,
        texture,
        0,
        layered ? GL_TRUE : GL_FALSE,
        0,
        access,
        img_format_to_gl_internal(descriptor.format)
    );
}

auto RenderCommandEncoder::draw(
    const u32 count,
    const u32 start,
    const u32 instance_count,
    const u32 instance_start
) -> void {
    ASSERT(m_tracked.bound_pipeline, "cannot draw without a bound graphics pipeline");

    const auto& pipeline = m_state.pipelines.details(*m_tracked.bound_pipeline);
    glDrawArraysInstancedBaseInstance(
        topology_to_gl(pipeline.topology),
        start,
        count,
        instance_count,
        instance_start
    );
}

auto RenderCommandEncoder::draw_indexed(const u32 count, const u32 start) -> void {
    ASSERT(m_tracked.bound_pipeline, "cannot draw without a bound graphics pipeline");
    ASSERT(m_tracked.bound_index, "cannot draw indexed without a bound index buffer");

    const auto& pipeline = m_state.pipelines.details(*m_tracked.bound_pipeline);
    const auto offset    = static_cast<std::uintptr_t>(
        start * m_tracked.bound_index->type.size_bytes()
    );

    glDrawElements(
        topology_to_gl(pipeline.topology),
        count,
        index_format_to_gl(m_tracked.bound_index->type),
        (void*)offset
    );
}

// == CommandBuffer ==

auto CommandBuffer::render_pass(
    [[maybe_unused]] const RenderPassDescriptor& descriptor,
    RenderPassFunction&& encode
) -> void {
    auto command_encoder = RenderCommandEncoder{m_state};
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
                img_format_to_gl_layout(descriptor.format),
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

    ASSERT(
        src_range.size() <= dst_descriptor.size.get() - dst_offset,
        "destination buffer is not large enough to hold requested data"
    );

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
