#include "command_executor.hpp"

#include <cstring>
#include <glad/gl.h>

#include "2iren/base.hpp"
#include "2iren/util/color.hpp"
#include "2iren/util/log.hpp"
#include "render_thread.hpp"
#include "util.hpp"

namespace siren {

// ============================================================================
// == MARK: Utilities
// ============================================================================

static constexpr auto get_buffer_slice(const std::vector<u8>& buffer, const usize offset, const usize size)
    -> std::span<const u8> {
    return std::span(buffer.data() + offset, size);
}

static constexpr auto extract_cmds(const RenderPass& pass, const std::vector<RenderCommand>& commands)
    -> std::span<const RenderCommand> {
    ASSERT(pass.start < commands.size(), "RenderPass has an invalid start index.");
    ASSERT(pass.start <= commands.size(), "RenderPass has more commands than available");
    return std::span(commands.data() + pass.start, pass.count);
}

// ============================================================================
// == MARK: Execution Loops
// ============================================================================


GlCommandExecutor::GlCommandExecutor(const RenderResourceState& state) : m_state(state) {}

auto GlCommandExecutor::execute(ResourceCommandBuffer&& resource_command_pacakge) -> void {
    for (const auto& cmd : resource_command_pacakge.commands) {
        switch (cmd.type) {
            case ResourceCommandType::UploadImage: {
                const auto& params = cmd.as<UploadImage>();
                execute_image_upload(std::move(cmd.command.upload_image_command),
                    get_buffer_slice(resource_command_pacakge.blob, params.data_offset, params.data_size));
                break;
            }
            case ResourceCommandType::UploadBuffer: {
                const auto& params = cmd.as<UploadBuffer>();
                execute_buffer_upload(std::move(cmd.command.upload_buffer_command),
                    get_buffer_slice(resource_command_pacakge.blob, params.blob_offset, params.data_size));
                break;
            }
            default: PANIC("Invalid ResourceCommandType encountered");
        }
    }
}

auto GlCommandExecutor::execute(RenderCommandBuffer&& render_command_package) -> void {
    for (const auto& pass : render_command_package.render_passes) {
        execute_pass(pass.descriptor, extract_cmds(pass, render_command_package.commands));
    }
}

// ============================================================================
// == MARK: Resource Commands
// ============================================================================

auto GlCommandExecutor::execute_image_upload(const UploadImage& cmd, const std::span<const u8> data_slice) const
    -> void {
    // just upload it all in one go, this should be fine even for cube maps
    const auto gl_handle = m_state.image_table.fetch(cmd.image_handle);
    const auto& desc     = m_state.image_table.details(cmd.image_handle).descriptor;

    switch (desc.dimension) {
        case ImageDimension::D1: {
            glTextureSubImage1D(gl_handle,
                0,
                0,
                static_cast<GLsizei>(desc.extent.width),
                gl::img_format_to_gl_layout(desc.format),
                GL_UNSIGNED_BYTE,
                data_slice.data());
            break;
        }

        case ImageDimension::D2: {
            glTextureSubImage2D(gl_handle,
                0,
                0,
                0,
                static_cast<GLsizei>(desc.extent.width),
                static_cast<GLsizei>(desc.extent.height),
                gl::img_format_to_gl_layout(desc.format),
                GL_UNSIGNED_BYTE,
                data_slice.data());
            break;
        }

        case ImageDimension::D3: {
            glTextureSubImage3D(gl_handle,
                0,
                0,
                0,
                0,
                static_cast<GLsizei>(desc.extent.width),
                static_cast<GLsizei>(desc.extent.height),
                static_cast<GLsizei>(desc.extent.depth_or_layers),
                gl::img_format_to_gl_layout(desc.format),
                GL_UNSIGNED_BYTE,
                data_slice.data());
            break;
        }

        case ImageDimension::Cube: {
            PANIC("Not implemented yet.");
        }
    }

    // generate mip map levels
    if (desc.mipmap_levels > 0) {
        glGenerateTextureMipmap(gl_handle);
    }
}

auto GlCommandExecutor::execute_buffer_upload(const UploadBuffer& cmd, const std::span<const u8> data_slice) const
    -> void {
    const auto gl_handle = m_state.buffer_table.fetch(cmd.buffer_handle);
    const auto& desc     = m_state.buffer_table.details(cmd.buffer_handle).descriptor;

    switch (desc.usage) {
        case BufferUsage::Static: {
            // create a temp staging buffer to copy data to the dest buffer
            GLuint staging_buffer;
            glCreateBuffers(1, &staging_buffer);
            glNamedBufferStorage(staging_buffer, static_cast<GLsizeiptr>(data_slice.size()), data_slice.data(), 0);

            // perform transfer
            glCopyNamedBufferSubData(staging_buffer,
                gl_handle,
                0,
                static_cast<GLintptr>(cmd.dest_offset),
                static_cast<GLsizeiptr>(data_slice.size()));

            // clean up staging buffer.
            glDeleteBuffers(1, &staging_buffer);
            break;
        }
        case BufferUsage::Dynamic: {
            glNamedBufferSubData(gl_handle,
                static_cast<GLintptr>(cmd.dest_offset),
                static_cast<GLsizeiptr>(data_slice.size()),
                data_slice.data());
            break;
        }
        case BufferUsage::Stream: {
            const auto [ptr, size] = m_state.buffer_table.details(cmd.buffer_handle).buffer_ptr;
            ASSERT(ptr != nullptr, "Stream Buffer mapped pointer is null!");
            ASSERT(size - cmd.dest_offset >= data_slice.size(), "Attempted to overwrite a Streamed mapped buffer!");
            std::memcpy(static_cast<u8*>(ptr) + cmd.dest_offset, data_slice.data(), data_slice.size());
            break;
        }
        default:
            ASSERT(
                false, "Invalid BufferUsage encountered. Cannot perform execute_buffer_upload on the OpenGL Backend");
    }
}

// ============================================================================
// == MARK: Render Commands
// ============================================================================

/// @todo: do we need to reset all state at the start of this function?
auto GlCommandExecutor::execute_pass(
    const RenderPassDescriptor& descriptor, const std::span<const RenderCommand> commands) const -> void {
    // todo: we assume there is always only a single color attachment, this is due to how RenderTarget is.
    // we also assume there is no depth stencil for now
    const GLuint image_id        = m_state.image_table.fetch(descriptor.target.color);
    const GLuint framebuffer     = m_state.framebuffer_cache.get_create_for(image_id);
    constexpr u32 num_colors     = 1;
    const bool has_depth_stencil = false;

    // first setup pass
    if (descriptor.begin_operation == BeginOperation::Clear) {
        RGBA color = descriptor.clear_color.value_or(RGBA::black());

        // clear color attachments
        if (descriptor.clear_color.has_value()) {
            color = descriptor.clear_color.value();
        }
        for (const auto color_index : range(num_colors)) {
            glClearNamedFramebufferfv(framebuffer, GL_COLOR, static_cast<GLint>(color_index), &color.r);
        }

        // clear depth stencil
        if (has_depth_stencil) {
            glClearNamedFramebufferfi(framebuffer, GL_DEPTH_STENCIL, 0, 1.f, 0);
        }
    }

    // restore default render settings

    // execute commands in the pass
    for (const auto& cmd : commands) {
        switch (cmd.type) {
            case RenderCommandType::BindGraphicsPipeline: {
                bind_graphics_pipeline(cmd.as<BindGraphicsPipeline>());
                break;
            }
            case RenderCommandType::SetViewport: {
                set_viewport(cmd.as<SetViewport>(), descriptor.target);
                break;
            }
            case RenderCommandType::BindVertexBuffer: {
                bind_vertex_buffer(cmd.as<BindVertexBuffer>());
                break;
            }
            case RenderCommandType::BindIndexBuffer: {
                bind_index_buffer(cmd.as<BindIndexBuffer>());
                break;
            }
            case RenderCommandType::BindUniformBuffer: {
                bind_uniform_buffer(cmd.as<BindUniformBuffer>());
                break;
            }
            case RenderCommandType::BindImage: {
                bind_image(cmd.as<BindImage>());
                break;
            }
            case RenderCommandType::DrawArrays: {
                draw_arrays(cmd.as<DrawArrays>());
                break;
            }
            case RenderCommandType::DrawIndexed: {
                draw_indexed(cmd.as<DrawIndexed>());
                break;
            }
        }
    }

    // clean up pass
}

auto GlCommandExecutor::bind_graphics_pipeline(const BindGraphicsPipeline& bind) const -> void {
    auto& gp_table           = m_state.graphics_pipeline_table;
    const auto va_handle     = gp_table.fetch(bind.pipeline_handle);
    const auto shader_handle = gp_table.details(bind.pipeline_handle).shader_program_handle;
    const auto& desc         = gp_table.details(bind.pipeline_handle).descriptor;

    m_tracked_state.active_pipeline = bind.pipeline_handle;
    m_tracked_state.active_vao      = va_handle;

    // bind the shader and vertex array == vertex layout
    glUseProgram(shader_handle);
    glBindVertexArray(va_handle);

    // set render state
    switch (desc.alpha_mode) {
        case AlphaMode::Opaque: {
            glEnable(GL_BLEND);
            break;
        }
        case AlphaMode::Blend: {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
        }
        case AlphaMode::Mask: {
            // shader has to handle discarding of fragments
            glEnable(GL_BLEND);
            break;
        }
    }

    glDepthFunc(gl::depth_func_to_gl(desc.depth_function));

    if (desc.back_face_culling) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        glDisable(GL_CULL_FACE);
    }

    if (desc.depth_write) {
        glDepthMask(GL_TRUE);
    } else {
        glDepthMask(GL_FALSE);
    }

    if (desc.depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    // draw mode aka PrimitiveTopology cannot be set here. Instead, we must
    // pass it in with each draw call.
}

auto GlCommandExecutor::set_viewport(const SetViewport& set_viewport, const RenderTarget& target) const -> void {
    // 2iren uses top left as origin, OpenGL uses bottom left, so we must convert
    // we need the target size for conversion
    // assume all attachments are the same size
    const auto target_height = m_state.image_table.details(target.color).descriptor.extent.height;

    const auto x      = set_viewport.x;
    const auto y      = target_height - (set_viewport.y + set_viewport.height);
    const auto width  = set_viewport.width;
    const auto height = set_viewport.height;
    glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

auto GlCommandExecutor::bind_vertex_buffer(const BindVertexBuffer& bind_vertex_buffer) const -> void {
    const auto vbo            = m_state.buffer_table.fetch(bind_vertex_buffer.vertex_buffer);
    const auto& pipeline_desc = m_state.graphics_pipeline_table.details(m_tracked_state.active_pipeline).descriptor;
    glVertexArrayVertexBuffer(m_tracked_state.active_vao,
        bind_vertex_buffer.slot,
        vbo,
        bind_vertex_buffer.offset,
        static_cast<GLsizei>(pipeline_desc.layout.stride));
}

auto GlCommandExecutor::bind_index_buffer(const BindIndexBuffer& bind_index_buffer) const -> void {
    const auto ibo             = m_state.buffer_table.fetch(bind_index_buffer.index_buffer);
    m_tracked_state.active_ibo = bind_index_buffer;
    glVertexArrayElementBuffer(m_tracked_state.active_vao, ibo);
}

auto GlCommandExecutor::bind_uniform_buffer(const BindUniformBuffer& bind_uniform_buffer) const -> void {
    const auto ubo = m_state.buffer_table.fetch(bind_uniform_buffer.uniform_buffer);
    glBindBufferBase(GL_UNIFORM_BUFFER, bind_uniform_buffer.slot, ubo);
}

auto GlCommandExecutor::bind_image(const BindImage& bind_image) const -> void {
    const auto img   = m_state.image_table.fetch(bind_image.image);
    const auto& desc = m_state.image_table.details(bind_image.image).descriptor;
    glBindImageTexture(bind_image.slot, img, 0, true, 0, GL_READ_ONLY, gl::img_format_to_gl_internal(desc.format));
}

auto GlCommandExecutor::draw_arrays(const DrawArrays& draw_arrays) const -> void {
    const auto& pl_desc = m_state.graphics_pipeline_table.details(m_tracked_state.active_pipeline).descriptor;
    const auto mode     = gl::topology_to_gl(pl_desc.topology);

    glDrawArrays(mode, static_cast<GLsizei>(draw_arrays.start), static_cast<GLsizei>(draw_arrays.count));
}

auto GlCommandExecutor::draw_indexed(const DrawIndexed& draw_indexed) const -> void {
    const auto& pl_desc = m_state.graphics_pipeline_table.details(m_tracked_state.active_pipeline).descriptor;
    const auto mode     = gl::topology_to_gl(pl_desc.topology);
    const auto type     = gl::index_format_to_gl(m_tracked_state.active_ibo.index_format);

    // because OpenGL is OpenGL, we pass in the first index as a void*. Its then
    // reinterpreted as a number.
    // also we must pass a byte offset, not an index offset.

    glDrawElements(mode,
        static_cast<GLsizei>(draw_indexed.index_count),
        type,
        reinterpret_cast<const void*>(draw_indexed.first_index * m_tracked_state.active_ibo.index_format.size_bytes()));
}

} // namespace siren
