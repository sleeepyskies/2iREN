#include "device.hpp"

#include <GLFW/glfw3.h>
#include <optional>
#include <utility>

#include "2iREN/core/assert.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/backend/opengl/mappings.hpp"
#include "2iREN/graphics/backend/opengl/resource_state.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/backend/opengl/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"

#include "2iREN/graphics/types.hpp"
#include "2iREN/utility/log.hpp"

#include "2iREN/window/window.hpp"

namespace siren {

using namespace opengl;

namespace {

auto fetch_limits() -> Limits {
    auto limits = Limits{};

    auto value = 0;

    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &value);
    limits.max_shader_storage_block_size = value;

    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &value);
    limits.uniform_buffer_offset_alignment = value;

    glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &value);
    limits.shader_storage_buffer_offset_alignment = value;

    return limits;
}

} // namespace

OpenGLDevice::OpenGLDevice() :
    Device(Backend::OpenGL), m_framebuffer_cache(m_state.images) {
    gladLoadGL(glfwGetProcAddress);
    m_limits = fetch_limits();
    log::info("opengl device made");
}

OpenGLDevice::~OpenGLDevice() {
    wait_idle();
}

auto OpenGLDevice::wait_idle() const noexcept -> void {
    glFinish();
}

auto OpenGLDevice::make_buffer(
    const BufferDescriptor& descriptor,
    std::optional<ByteBufferView> initial
) -> Buffer {
    GLuint buffer;
    glCreateBuffers(1, &buffer);

    if (const auto label = descriptor.label; label) {
        glObjectLabel(GL_BUFFER, buffer, label->size(), label->data());
    }

    const auto* data = initial.has_value() ? initial.value().data() : nullptr;
    glNamedBufferStorage(
        buffer,
        static_cast<GLsizeiptr>(descriptor.size.get()),
        data,
        buffer_bitfield(descriptor.memory_usage)
    );

    const auto handle = m_state.buffers.reserve_link(buffer, BufferDescriptor{descriptor});

    log::trace("{} made", handle);
    return Buffer{this, handle};
}

auto OpenGLDevice::make_image(const ImageDescriptor& descriptor) -> Image {
    // NOTE: opengl has no way to restrict the access of texture memory access,
    // so for the opengl backend we enforce this restriction via cpu side checks.
    ASSERT(
        descriptor.extent.x > 0 and descriptor.extent.y > 0 and descriptor.extent.z > 0,
        "cannot make an empty image"
    );

    const auto target          = img_to_target_gl(descriptor.extent, descriptor.dimension);
    const auto internal_format = img_format_to_gl_internal(descriptor.format);
    const auto& extent         = descriptor.extent;

    GLuint image;
    glCreateTextures(target, 1, &image);

    if (const auto label = descriptor.label; label) {
        glObjectLabel(GL_TEXTURE, image, label->size(), label->data());
    }

    switch (target) {
        case GL_TEXTURE_1D:
            glTextureStorage1D(
                image,
                1,
                internal_format,
                static_cast<GLsizei>(extent.x)
            );
            break;
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
            glTextureStorage2D(
                image,
                1,
                internal_format,
                static_cast<GLsizei>(extent.x),
                static_cast<GLsizei>(extent.y)
            );
            break;
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            glTextureStorage3D(
                image,
                1,
                internal_format,
                static_cast<GLsizei>(extent.x),
                static_cast<GLsizei>(extent.y),
                static_cast<GLsizei>(extent.z)
            );
            break;
        default: PANIC("Unsupported texture target");
    }

    const auto handle = m_state.images.reserve_link(image, ImageDetails{.descriptor = descriptor});

    log::trace("{} made", handle);
    return Image{this, handle};
}

auto OpenGLDevice::make_sampler(const SamplerDescriptor& descriptor) -> Sampler {
    GLuint sampler;
    glCreateSamplers(1, &sampler);

    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, img_filter_to_gl(descriptor.min_filter));
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, img_filter_to_gl(descriptor.mag_filter));

    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, img_wrap_to_gl(descriptor.s_wrap));
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, img_wrap_to_gl(descriptor.t_wrap));
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, img_wrap_to_gl(descriptor.r_wrap));

    if (const auto label = descriptor.label; label.has_value()) {
        glObjectLabel(GL_SAMPLER, sampler, label->length(), label->data());
    }

    const auto handle = m_state.samplers.reserve_link(sampler, SamplerDescriptor{descriptor});

    log::trace("{} made", handle);
    return Sampler{this, handle};
}

auto OpenGLDevice::make_shader(const ShaderDescriptor& descriptor) -> Shader {
    ASSERT(descriptor.source.contains(ShaderStage::Vertex), "2iREN requires a vertex shader");
    ASSERT(descriptor.source.contains(ShaderStage::Fragment), "2iREN requires a fragment shader");

    // NOTE: debug callbacks dont handle shader compilation
    GLint success;
    char err_info[512];

    std::vector<GLuint> shader_ids = {};
    shader_ids.reserve(descriptor.source.size());

    // compile shaders
    for (const auto& [stage, stage_data] : descriptor.source) {
        const GLuint shader = glCreateShader(shader_stage_to_gl(stage));
        const char* raw     = stage_data.source.c_str();

        glShaderSource(shader, 1, &raw, nullptr);
        glCompileShader(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, nullptr, err_info);
            log::warn("{} Shader compilation from failed with error message: {}", stage, err_info);
        }

        if (const auto label = stage_data.label; label.has_value()) {
            glObjectLabel(GL_SHADER, shader, label->size(), label->data());
        }

        shader_ids.push_back(shader);
    }

    // link shaders
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

    std::ranges::for_each(shader_ids, glDeleteShader);

    if (const auto label = descriptor.label; label.has_value()) {
        glObjectLabel(GL_PROGRAM, program, label->size(), label->data());
    }

    const auto handle = m_state.shaders.reserve_link(program, ShaderDescriptor{descriptor});

    log::trace("{} made", handle);
    return Shader{this, handle};
}

// FIXME: this doesnt work at all, need to figure out a way to handle swapchains in gl
auto OpenGLDevice::make_swapchain(const Window& window, const SwapchainDescriptor& descriptor)
    -> Swapchain {
    const auto info = SwapchainInfo {
        .extent       = descriptor.extent.value_or(window.framebuffer_extent()),
        .vsync        = descriptor.vsync.value_or(true),
        .image_format = descriptor.image_format.value_or(ImageFormat::RGBA8),
    };

    const auto handle = m_state.swapchains.reserve_link(
        nullptr,
        SwapchainDetails{
            .info = info,
            .native_handle = window.native_handle(),
            .acquired_image = std::nullopt,
        }
    );

    reconfigure_swapchain(handle, descriptor);

    log::trace("{} made", handle);
    return Swapchain{this, handle};
}

auto OpenGLDevice::reconfigure_swapchain(
    SwapchainHandle handle,
    const SwapchainDescriptor& new_values
) -> void {
    auto& details = m_state.swapchains.details(handle);

    if (new_values.extent) {
        // need to call glViewport once starting a new render pass tho :D
        details.info.extent = *new_values.extent;
    }

    if (new_values.vsync) {
        glfwSwapInterval(*new_values.vsync);
    }

    if (const auto format = new_values.image_format; new_values.image_format.has_value()) {
        ASSERT(
            format.value() == details.info.image_format,
            "cannot update the image format on opengl backend"
        );
    }

    log::trace("updated swapchain {}", handle);
}

auto OpenGLDevice::make_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
    -> GraphicsPipeline {
    // FIXME: we shouldn't crash here on failure imo
    ASSERT(
        m_state.shaders.fetch(descriptor.shader) != 0, 
        "cannot make GraphicsPipeline with invalid Shader"
    );

    GLuint vao;
    glCreateVertexArrays(1, &vao);

    // optionally label the vertex array
    if (const auto label = descriptor.label; label.has_value()) {
        glObjectLabel(GL_VERTEX_ARRAY, vao, label->size(), label->data());
    }

    for (u32 i = 0; i < descriptor.layout.attributes.size(); i++) {
        auto& attribute = descriptor.layout.attributes[i];

        // enables some element aka the layout(location = n) shader side
        glEnableVertexArrayAttrib(vao, i);

        // describe the element
        glVertexArrayAttribFormat(
            vao,
            i,
            attribute.size,
            siren_datatype_to_gl(attribute.type),
            false,
            // attribute.normalized,
            static_cast<GLuint>(attribute.offset)
        );

        // NOTE:
        // we always bind the vao to index 0. this is because 2iREN
        // assumes a single vertex buffer per graphics pipeline.
        // if a pipeline were to expect multiple, we would need to
        // update how we handle binding vaos here.
        glVertexArrayAttribBinding(vao, i, 0);
    }

    const auto handle = m_state.pipelines.reserve_link(
        vao,
        GraphicsPipelineDescriptor{descriptor}
    );

    log::trace("{} made", handle);
    return GraphicsPipeline{this, handle};
}

auto OpenGLDevice::make_query(const QueryDescriptor&) -> Query {
    // TODO: how do we want to impl queries in gl? using query buffer instead?
    UNIMPLEMENTED();
    /*
    GLuint query;
    glGenQueries(1, &query);
    const auto handle = m_state.queries.link(handle, query, GlQueryDetails{.descriptor =
    descriptor}); log::trace("{} made", handle); return Query{this, handle};
    */
}

auto OpenGLDevice::make_command_buffer() const noexcept -> std::unique_ptr<::siren::CommandBuffer> {
    return std::make_unique<opengl::CommandBuffer>(m_state, m_framebuffer_cache);
}

auto OpenGLDevice::destroy_buffer(const BufferHandle handle) -> void {
    const auto glhandle = m_state.buffers.fetch_release(handle);
    glDeleteBuffers(1, &glhandle);
    log::trace("{} deleted", handle);
}

auto OpenGLDevice::destroy_image(const ImageHandle handle) -> void {
    m_framebuffer_cache.invalidate(handle);
    const auto glhandle = m_state.images.fetch_release(handle);
    glDeleteTextures(1, &glhandle);
    log::trace("{} deleted.", handle);
}

auto OpenGLDevice::destroy_sampler(const SamplerHandle handle) -> void {
    const auto glhandle = m_state.samplers.fetch_release(handle);
    glDeleteSamplers(1, &glhandle);
    log::trace("{} deleted.", handle);
}

auto OpenGLDevice::destroy_shader(const ShaderHandle handle) -> void {
    const auto glhandle = m_state.shaders.fetch_release(handle);
    glDeleteProgram(glhandle);
    log::trace("{} deleted.", handle);
}

auto OpenGLDevice::destroy_swapchain(const SwapchainHandle handle) -> void {
    m_state.swapchains.fetch_release(handle);
    log::trace("{} deleted.", handle);
}

auto OpenGLDevice::destroy_graphics_pipeline(const GraphicsPipelineHandle handle) -> void {
    const auto glhandle = m_state.pipelines.fetch_release(handle);
    glDeleteVertexArrays(1, &glhandle);
    log::trace("{} deleted.", handle);
}

auto OpenGLDevice::destroy_query(const QueryHandle) -> void {
    // TODO: how to impl queries
    /*
    const auto glhandle = m_state.query_table.fetch_release(handle);
    glDeleteQueries(1, &glhandle);
    log::trace("{} deleted.", handle);
    */
}

auto OpenGLDevice::buffer_descriptor(const BufferHandle handle) const -> const BufferDescriptor& {
    return m_state.buffers.details(handle);
}

auto OpenGLDevice::image_descriptor(const ImageHandle handle) const -> const ImageDescriptor& {
    return m_state.images.details(handle).descriptor;
}

auto OpenGLDevice::sampler_descriptor(const SamplerHandle handle) const
    -> const SamplerDescriptor& {
    return m_state.samplers.details(handle);
}

auto OpenGLDevice::shader_descriptor(const ShaderHandle handle) const -> const ShaderDescriptor& {
    return m_state.shaders.details(handle);
}

auto OpenGLDevice::graphics_pipeline_descriptor(const GraphicsPipelineHandle handle) const
    -> const GraphicsPipelineDescriptor& {
    return m_state.pipelines.details(handle);
}

auto OpenGLDevice::query_descriptor(const QueryHandle) const -> const QueryDescriptor& {
    UNIMPLEMENTED();
    // TODO: impl this shit nigga
    //
    // return m_state.query_table.details(handle).descriptor;
}

auto OpenGLDevice::swapchain_info(SwapchainHandle handle) const -> SwapchainInfo {
    return m_state.swapchains.details(handle).info;
}

auto OpenGLDevice::acquire_next_swapchain_image(SwapchainHandle handle) -> ImageHandle {
    auto& details = m_state.swapchains.details(handle);

    ASSERT(!details.acquired_image.has_value());

    details.acquired_image = m_state.images.reserve_link(0, ImageDetails{
        .descriptor = ImageDescriptor{
            .label = "Swapchain Image",
            .format = details.info.image_format,
            .extent = details.info.extent.to_extent3(),
            /// TODO: should we enabled shader read and write here??
            .flags = ImageFlags::make(ImageFlag::RenderAttachment, ImageFlag::ShaderRead, ImageFlag::ShaderWrite),
        },
        .default_framebuffer = true,
    });

    return *details.acquired_image;
}

auto OpenGLDevice::submit(std::unique_ptr<::siren::CommandBuffer>&&) const -> void {
    // TODO: 
    // atm, we perform opengl commands as they are recorded, so here theres nothing to do :D
    // however, this isnt the best. eventually a vk backend would replace opengl and then we can 
    // have consistent behaviour across backends
}

auto OpenGLDevice::present(const SwapchainHandle handle) -> void {
    auto& details = m_state.swapchains.details(handle);

    ASSERT(details.acquired_image.has_value(), "cannot present swapchain without an acquired image");

    glfwSwapBuffers(details.native_handle);
    details.acquired_image.reset(); // calls destroy_image in dtor for us
}

auto OpenGLDevice::present(SwapchainHandle handle, std::unique_ptr<::siren::CommandBuffer>&& cmds) -> void {
    submit(std::move(cmds)); // does nothing, work is already done by this point
    present(handle);
}

auto OpenGLDevice::read_buffer(BufferHandle buffer) const -> ByteBuffer {
    const auto& descriptor = m_state.buffers.details(buffer);
    ASSERT(descriptor.memory_usage == MemoryUsage::Shared, "cannot read from private buffer");

    const auto glhandle = m_state.buffers.fetch(buffer);
    auto byte_buffer    = ByteBuffer::with_size_bytes(descriptor.size);
    glGetNamedBufferSubData(glhandle, 0, byte_buffer.size_bytes(), byte_buffer.data());

    return byte_buffer;
}

} // namespace siren
