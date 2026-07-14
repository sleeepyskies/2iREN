#include "device.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <libassert/assert.hpp>

#include "2iren/rhi/resource_command.hpp"
#include "command_executor.hpp"
#include "render_thread.hpp"

#include "2iren/rhi/resources/buffer.hpp"
#include "2iren/rhi/resources/graphics_pipeline.hpp"
#include "2iren/rhi/resources/image.hpp"
#include "2iren/rhi/resources/sampler.hpp"
#include "2iren/rhi/resources/shader.hpp"
#include "2iren/rhi/resources/swapchain.hpp"

#include "2iren/util/log.hpp"
#include "2iren/window.hpp"
#include "util.hpp"

namespace siren {

// todo: add logging

constexpr GLuint GL_DEFAULT_FRAMEBUFFER = 0;

using namespace siren;

/// helper to create an optional label of form "prefix-suffix"
static auto make_label(const std::optional<std::string>& prefix, const std::string_view suffix)
    -> std::optional<std::string> {
    if (prefix) {
        return *prefix + "-" + std::string(suffix);
    }
    return std::nullopt;
}

auto FramebufferCache::get_create_for(const RenderTarget& target) -> GLuint {
    // first search cache
    const Key key{
        .colors        = target.colors | views::transform(&Attachment::image) | ranges::to<std::vector>(),
        .depth_stencil = target.depth_stencil.transform([](auto a) { return a.image; }).value_or(NullHandle),
    };
    if (const auto it = m_cache.find(key); it != m_cache.end()) {
        return it->second;
    }

    // otherwise create a new framebuffer
    const auto fb = create_framebuffer(target);
    m_cache[key]  = fb;
    return fb;
}

auto FramebufferCache::Hasher::operator()(const Key& key) const -> usize {
    // ty mr chatgpt, idk nun bout this
    usize hash   = 0;
    auto combine = [&hash](const usize value) { hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2); };
    for (const auto image : key.colors)
        combine(image.hash());
    if (key.depth_stencil.is_valid()) {
        combine(key.depth_stencil.hash());
    }
    return hash;
}

auto FramebufferCache::create_framebuffer(const RenderTarget& target) -> GLuint {
    // note that this is all happening inside the render thread, so we can do as many gl calls as we want directly :D
    GLuint framebuffer;
    glCreateFramebuffers(1, &framebuffer);

    for (const auto [index, attachment] : views::enumerate(target.colors)) {
        const auto image_id = m_image_table.fetch(attachment.image);
        glNamedFramebufferTexture(framebuffer, GL_COLOR_ATTACHMENT0 + index, image_id, 0);
    }

    if (target.depth_stencil.has_value()) {
        const auto image_id = m_image_table.fetch(target.depth_stencil->image);
        glNamedFramebufferTexture(framebuffer, GL_DEPTH_STENCIL_ATTACHMENT, image_id, 0);
    }

    // makes the buffers drawable
    std::vector<GLenum> draw_buffers;
    draw_buffers.reserve(target.colors.size());
    for (const usize index : range(target.colors.size())) {
        draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + index);
    }
    glNamedFramebufferDrawBuffers(framebuffer, draw_buffers.size(), draw_buffers.data());

    ASSERT(glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
        "Framebuffer could not be created.");

    return framebuffer;
}

GlDevice::GlDevice(const Window& window) : m_primary_window(window), m_render_thread(window) {}

GlDevice::~GlDevice() {}

auto GlDevice::wait_until_idle() const noexcept -> void { m_render_thread.wait_until_idle(); }

auto GlDevice::create_buffer(const BufferDescriptor& descriptor) -> Buffer {
    ASSERT(descriptor.size > 0, "Cannot legally allocate empty buffer (sorry).");
    const auto buffer_handle = m_state.buffer_table.reserve();

    // todo: we do a copy of the whole initial buffer here, not great.

    m_render_thread.spawn([buffer_handle, descriptor, this] {
        // create buffer id
        GLuint buf;
        glCreateBuffers(1, &buf);

        // optionally name it
        if (descriptor.label.has_value()) {
            glObjectLabel(
                GL_BUFFER, buf, static_cast<GLsizei>(descriptor.label.value().size()), descriptor.label.value().data());
        }

        const auto flags = gl::buffer_usage_to_flags_gl(descriptor.usage);

        // if data was passed, directly upload it
        const u8* data = nullptr;
        if (descriptor.data.has_value()) {
            data = descriptor.data.value().data();
        }
        glNamedBufferStorage(buf, static_cast<GLsizeiptr>(descriptor.size), data, flags);

        // if the buffer is streamed, we also need to store a mapping pointer
        MappedBufferPtr mapped_buffer;
        mapped_buffer.size = descriptor.size;
        if (descriptor.usage == BufferUsage::Stream) {
            mapped_buffer.ptr = glMapNamedBufferRange(buf,
                0,
                static_cast<GLsizeiptr>(descriptor.size),
                GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
        }

        // link proxy handle to opengl handle
        this->m_state.buffer_table.link(
            buffer_handle, buf, GlBufferDetails{.descriptor = descriptor, .buffer_ptr = mapped_buffer});
    });

    log::trace("Created buffer {}", buffer_handle);
    return Buffer{this, buffer_handle};
}

auto GlDevice::destroy_buffer(const BufferHandle handle) -> void {
    const auto api_handle = m_state.buffer_table.fetch(handle);
    if (api_handle != 0) {
        m_delete_queue.push_back({api_handle, ResourceType::Buffer});
    }
    m_state.buffer_table.release(handle);
    log::trace("Queued buffer {} for cleanup", handle);
}

auto GlDevice::create_image(const ImageDescriptor& descriptor) -> Image {
    ASSERT(descriptor.extent.width > 0 && descriptor.extent.height > 0 && descriptor.extent.depth_or_layers > 0,
        "Cannot create an empty image.");
    const auto image_handle = m_state.image_table.reserve();
    m_render_thread.spawn([image_handle, descriptor, this] {
        const auto target = gl::img_to_target_gl(descriptor.extent, descriptor.dimension);

        // create the image
        GLuint img;
        glCreateTextures(target, 1, &img);

        // optionally name it
        if (descriptor.label.has_value()) {
            glObjectLabel(GL_TEXTURE,
                img,
                static_cast<GLsizei>(descriptor.label.value().size()),
                descriptor.label.value().data());
        }

        const auto internal_format = gl::img_format_to_gl_internal(descriptor.format);
        const auto& extent         = descriptor.extent;

        // allocate enough memory
        switch (target) {
            case GL_TEXTURE_1D:
                glTextureStorage1D(img,
                    static_cast<GLsizei>(descriptor.mipmap_levels),
                    internal_format,
                    static_cast<GLsizei>(extent.width));
                break;
            case GL_TEXTURE_1D_ARRAY:
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP: // Cubemaps use 2D storage
                glTextureStorage2D(img,
                    static_cast<GLsizei>(descriptor.mipmap_levels),
                    internal_format,
                    static_cast<GLsizei>(extent.width),
                    static_cast<GLsizei>(extent.height));
                break;
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_3D:
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                glTextureStorage3D(img,
                    static_cast<GLsizei>(descriptor.mipmap_levels),
                    internal_format,
                    static_cast<GLsizei>(extent.width),
                    static_cast<GLsizei>(extent.height),
                    static_cast<GLsizei>(extent.depth_or_layers));
                break;
            default: PANIC("Unsupported texture target");
        }

        // assign the proxy handle to the real handle
        this->m_state.image_table.link(image_handle, img, GlImageDetails{.descriptor = descriptor});
    });

    log::trace("Created image {}", image_handle);
    return Image{this, image_handle};
}

auto GlDevice::destroy_image(const ImageHandle handle) -> void {
    const auto api_handle = m_state.image_table.fetch(handle);
    if (api_handle != 0) {
        m_delete_queue.push_back({api_handle, ResourceType::Image});
    }
    m_state.image_table.release(handle);
    log::trace("Queued image {} for cleanup", handle);
}

auto GlDevice::create_sampler(const SamplerDescriptor& descriptor) -> Sampler {
    const auto sampler_handle = m_state.sampler_table.reserve();

    m_render_thread.spawn([descriptor, sampler_handle, this]() {
        GLuint sampler;
        glCreateSamplers(1, &sampler);
        glSamplerParameteri(sampler,
            GL_TEXTURE_MIN_FILTER,
            static_cast<GLint>(gl::min_img_filter_to_gl(descriptor.min_filter, descriptor.mipmap_filter)));
        glSamplerParameteri(
            sampler, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(gl::img_filter_to_gl(descriptor.min_filter)));

        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, static_cast<GLint>(gl::img_wrap_to_gl(descriptor.s_wrap)));
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, static_cast<GLint>(gl::img_wrap_to_gl(descriptor.t_wrap)));
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, static_cast<GLint>(gl::img_wrap_to_gl(descriptor.r_wrap)));

        glSamplerParameterf(sampler, GL_TEXTURE_MIN_LOD, descriptor.lod_min);
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_LOD, descriptor.lod_max);

        if (descriptor.border_color.has_value()) {
            glSamplerParameterfv(sampler,
                GL_TEXTURE_BORDER_COLOR,
                reinterpret_cast<const GLfloat*>(glm::value_ptr(descriptor.border_color.value().to_vec4())));
        }

        glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, gl::img_compare_mode_to_gl(descriptor.compare_mode));
        glSamplerParameteri(
            sampler, GL_TEXTURE_COMPARE_FUNC, static_cast<GLint>(gl::img_compare_fn_to_gl(descriptor.compare_fn)));

        this->m_state.sampler_table.link(sampler_handle, sampler, GlSamplerDetails{.descriptor = descriptor});
    });

    log::trace("Created sampler {}", sampler_handle);
    return Sampler{this, sampler_handle};
}

auto GlDevice::destroy_sampler(const SamplerHandle handle) -> void {
    const auto api_handle = m_state.sampler_table.fetch(handle);
    if (api_handle != 0) {
        m_delete_queue.push_back({api_handle, ResourceType::Sampler});
    }
    m_state.sampler_table.release(handle);
    log::trace("Queued sampler {} for cleanup", handle);
}

auto GlDevice::create_shader(const ShaderDescriptor& descriptor) -> Shader {
    ASSERT(descriptor.source.contains(ShaderStage::Vertex), "Cannot create a Shader without a Vertex Shader");
    ASSERT(descriptor.source.contains(ShaderStage::Fragment), "Cannot create a Shader without a Fragment Shader");

    const auto shader_handle = m_state.shader_table.reserve();

    m_render_thread.spawn([descriptor, shader_handle, this] {
        // debug callbacks dont handle shader compilation
        GLint success;
        char err_info[512];

        std::vector<GLuint> shader_ids;
        shader_ids.reserve(descriptor.source.size());

        // process all stages, we can be sure we have at least vertex + fragment here
        for (const auto& [stage, stage_data] : descriptor.source) {
            const GLuint shader = glCreateShader(gl::shader_stage_to_gl(stage));
            const char* raw     = stage_data.source.c_str();

            // compile and check status of shader
            glShaderSource(shader, 1, &raw, nullptr);
            glCompileShader(shader);

            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 512, nullptr, err_info);
                log::warn("{} Shader compilation failed with error message: {}", stage, err_info);
            }

            // optionally label the shader
            if (stage_data.label.has_value()) {
                glObjectLabel(GL_SHADER,
                    shader,
                    static_cast<GLsizei>(stage_data.label.value().size()),
                    stage_data.label.value().c_str());
            }

            shader_ids.push_back(shader);
        }

        // link all stages together into a program
        const auto program = glCreateProgram();
        for (const auto& shader : shader_ids) {
            glAttachShader(program, shader);
        }
        glLinkProgram(program);

        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(program, 512, nullptr, err_info);
            log::warn("Shader linking failed with error message: {}", err_info);
        }

        // delete all shaders since they are linked to program
        for (const auto& shader : shader_ids) {
            glDeleteShader(shader);
        }

        // cache uniforms
        i32 uniform_count = 0;
        std::unordered_map<std::string, GLint> cache;

        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniform_count);
        if (uniform_count != 0) {
            i32 max_name_length = 0;
            GLsizei length      = 0;
            GLsizei count       = 0;
            GLenum type         = GL_NONE;
            glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_name_length);
            const auto uniform_name = std::make_unique<char[]>(static_cast<usize>(max_name_length));

            for (i32 i = 0; i < uniform_count; i++) {
                glGetActiveUniform(
                    program, static_cast<GLuint>(i), max_name_length, &length, &count, &type, uniform_name.get());
                const i32 location = glGetUniformLocation(program, uniform_name.get());
                if (location != -1) {
                    cache[std::string(uniform_name.get(), static_cast<usize>(length))] = location;
                }
            }
        }

        // optionally label the shader program
        if (descriptor.label.has_value()) {
            glObjectLabel(GL_PROGRAM,
                program,
                static_cast<GLsizei>(descriptor.label.value().size()),
                descriptor.label.value().data());
        }

        this->m_state.shader_table.link(
            shader_handle, program, GlShaderDetails{.descriptor = descriptor, .uniform_cache = cache});
    });

    log::trace("Created shader {}", shader_handle);
    return Shader{this, shader_handle};
}

auto GlDevice::destroy_shader(const ShaderHandle handle) -> void {
    const auto api_handle = m_state.shader_table.fetch(handle);
    if (api_handle != 0) {
        m_delete_queue.push_back({api_handle, ResourceType::Shader});
    }
    m_state.shader_table.release(handle);
    log::trace("Queued shader {} for cleanup", handle);
}

auto GlDevice::create_swapchain(const SwapchainDescriptor& descriptor) -> Swapchain {
    // so OpenGL doesn't expose any concept of a swapchain, so the opengl swapchain in 2iren
    // is just an offscreen image. on Device::present() we just blit this image to window.

    const auto swapchain_handle = m_state.swapchain_table.reserve();

    m_render_thread.spawn([this, swapchain_handle = swapchain_handle, descriptor = descriptor]() -> void {
        // first create the offscreen image
        i32 width, height;
        glfwGetFramebufferSize(m_primary_window.handle(), &width, &height);
        auto image = create_image({
            .label         = make_label(descriptor.label, "Swapchain Backbuffer").value_or("Swapchain Backbuffer"),
            .format        = ImageFormat::RGBA8,
            .extent        = ImageExtent{.width = (u32)width, .height = (u32)height, .depth_or_layers = 1},
            .dimension     = ImageDimension::D2,
            .mipmap_levels = 1,
        });

        Attachment attachment{.image = image.handle()};

        m_state.swapchain_table.link(swapchain_handle,
            nullptr,
            GlSwapchainDetails{
                .descriptor    = descriptor,
                .native_handle = m_primary_window.handle(),
                .target =
                    GlSwapchainDetails::Target{
                        .render_target = RenderTarget{.colors = {std::move(attachment)}, .depth_stencil = std::nullopt},
                        .image         = std::move(image),
                    },
            });

        glfwSwapInterval(descriptor.vsync);
    });

    log::trace("Created Swapchain {}", swapchain_handle);
    return Swapchain{this, swapchain_handle};
}

auto GlDevice::destroy_swapchain(const SwapchainHandle handle) -> void {
    m_state.swapchain_table.release(handle);
    log::trace("Queued swapchain {} for cleanup", handle);
}

auto GlDevice::create_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor) -> GraphicsPipeline {
    const auto pipeline_handle = m_state.graphics_pipeline_table.reserve();

    m_render_thread.spawn([pipeline_handle, descriptor, this] {
        // check the shader exists
        const auto program_handle = this->m_state.shader_table.fetch(descriptor.shader);
        ASSERT(program_handle != 0, "Cannot create GraphicsPipeline with invalid Shader.");

        GLuint vertex_array;
        glCreateVertexArrays(1, &vertex_array);

        // optionally label the vertex array
        if (descriptor.label.has_value()) {
            glObjectLabel(GL_VERTEX_ARRAY,
                vertex_array,
                static_cast<GLsizei>(descriptor.label.value().size()),
                descriptor.label.value().data());
        }

        for (const auto& [index, attribute] : descriptor.layout.components | views::enumerate) {
            // enables some element aka the layout(location = n) shader side
            glEnableVertexArrayAttrib(vertex_array, static_cast<GLuint>(index));

            // describe the element
            // todo: stride should be in the somewhere else now :D
            glVertexArrayAttribFormat(vertex_array,
                static_cast<GLuint>(index),
                static_cast<GLint>(attribute.size),
                gl::siren_datatype_to_gl(attribute.type),
                false, // attribute.normalized,
                static_cast<GLuint>(attribute.offset));

            // link all attributes to binding index 0 for this vao.
            // use of multiple binding indices bay be useful when data
            // is spread over multiple buffers.
            //
            // the exception is the index buffer, as the vao gets
            // a special slot for this
            glVertexArrayAttribBinding(vertex_array, static_cast<GLuint>(index), 0);
        }

        m_state.graphics_pipeline_table.link(
            pipeline_handle, vertex_array, GlGraphicsPipelineDetails{.descriptor = descriptor});
    });

    log::trace("Created graphics pipeline {}", pipeline_handle);
    return GraphicsPipeline{this, pipeline_handle};
}

auto GlDevice::destroy_graphics_pipeline(const GraphicsPipelineHandle handle) -> void {
    const auto api_handle = m_state.graphics_pipeline_table.fetch(handle);
    if (api_handle != 0) {
        m_delete_queue.push_back({api_handle, ResourceType::GraphicsPipeline});
    }
    m_state.graphics_pipeline_table.release(handle);
    log::trace("Queued graphics pipeline {} for cleanup", handle);
}

auto GlDevice::flush_delete_queue() -> void {
    if (m_delete_queue.empty()) {
        return;
    }

    m_render_thread.spawn([delete_queue = std::move(m_delete_queue)] {
        for (const auto& [handle, type] : delete_queue) {
            switch (type) {
                case ResourceType::Buffer: {
                    glDeleteBuffers(1, &handle);
                    break;
                }
                case ResourceType::Image: {
                    glDeleteTextures(1, &handle);
                    break;
                }
                case ResourceType::Sampler: {
                    glDeleteSamplers(1, &handle);
                    break;
                }
                case ResourceType::Framebuffer: {
                    glDeleteFramebuffers(1, &handle);
                    break;
                }
                case ResourceType::Shader: {
                    glDeleteProgram(handle);
                    break;
                }
                case ResourceType::GraphicsPipeline: {
                    glDeleteVertexArrays(1, &handle);
                    break;
                }
            }
        }
    });

    m_delete_queue.clear();
}

auto GlDevice::record_resource_commands() const -> ResourceCommandRecorder { return ResourceCommandRecorder{}; }

auto GlDevice::record_render_commands() const -> RenderCommandRecorder { return RenderCommandRecorder{}; }

auto GlDevice::submit(ResourceCommandBuffer&& command_buffer) const -> void {
    m_render_thread.spawn([this, cmds = std::move(command_buffer)]() mutable -> void {
        GlCommandExecutor executor{this->m_state};
        executor.execute(std::move(cmds));
    });
}

auto GlDevice::submit(RenderCommandBuffer&& command_buffer) const -> void {
    m_render_thread.spawn([this, cmds = std::move(command_buffer)]() mutable -> void {
        GlCommandExecutor executor{this->m_state};
        executor.execute(std::move(cmds));
    });
}

auto GlDevice::buffer_descriptor(const BufferHandle handle) const -> const BufferDescriptor& {
    return m_state.buffer_table.details(handle).descriptor;
}

auto GlDevice::image_descriptor(const ImageHandle handle) const -> const ImageDescriptor& {
    return m_state.image_table.details(handle).descriptor;
}

auto GlDevice::sampler_descriptor(const SamplerHandle handle) const -> const SamplerDescriptor& {
    return m_state.sampler_table.details(handle).descriptor;
}

auto GlDevice::shader_descriptor(const ShaderHandle handle) const -> const ShaderDescriptor& {
    return m_state.shader_table.details(handle).descriptor;
}

auto GlDevice::graphics_pipeline_descriptor(const GraphicsPipelineHandle handle) const
    -> const GraphicsPipelineDescriptor& {
    return m_state.graphics_pipeline_table.details(handle).descriptor;
}

auto GlDevice::swapchain_descriptor(const SwapchainHandle handle) const -> const SwapchainDescriptor& {
    return m_state.swapchain_table.details(handle).descriptor;
}

auto GlDevice::acquire_next_swapchain_target(const SwapchainHandle handle) const -> ImageHandle {
    return m_state.swapchain_table.details(handle).target->image.handle();
}

auto GlDevice::present(const SwapchainHandle handle) const -> void {
    // blit the offscreen image to the default framebuffer, then swap buffers
    auto* window         = m_state.swapchain_table.details(handle).native_handle;
    const auto& target   = m_state.swapchain_table.details(handle).target->render_target;
    const auto [w, h, _] = m_state.image_table.details(target.colors[0].image).descriptor.extent;

    // basically, we just blit swapchain image fbo to default fbo
    m_render_thread.spawn([this, window, target, w, h] -> void {
        // clang-format off
        const auto offscreen_fb  = m_state.framebuffer_cache.get_create_for(target);
        glBlitNamedFramebuffer(
            /* from */ offscreen_fb, /* to */ GL_DEFAULT_FRAMEBUFFER,
            0, 0, w, h,
            0, 0, w, h,
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
        // clang-format on

        glfwSwapBuffers(window);
    });
}

auto GlDevice::blit(const ImageHandle source, const ImageHandle destination) const -> void {
    // opengl doesn't have image blitting, so we get_create() cached fbos for images and then blit between the fbos.
    const auto source_id      = m_state.image_table.fetch(source);
    const auto destination_id = m_state.image_table.fetch(destination);
    const auto& source_desc   = m_state.image_table.details(source).descriptor;

    m_render_thread.spawn([source_id, destination_id, source_desc = source_desc]() -> void {
        // clang-format off
        glCopyImageSubData(
            source_id, GL_TEXTURE_2D, /*level*/ 0, 0, 0, 0,
            destination_id, GL_TEXTURE_2D, /*level*/ 0, 0, 0, 0,
            source_desc.extent.width, source_desc.extent.height, 1
        );
        // clang-format on
    });
}

auto GlDevice::limits() const -> Limits { return Limits{}; }

} // namespace siren
