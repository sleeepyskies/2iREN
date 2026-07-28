#pragma once

#include <glad/gl.h>
#include <unordered_map>

#include "2iren/rhi/device.hpp"
#include "2iren/rhi/resources/buffer.hpp"
#include "2iren/rhi/resources/graphics_pipeline.hpp"
#include "2iren/rhi/resources/image.hpp"
#include "2iren/rhi/resources/sampler.hpp"
#include "2iren/rhi/resources/shader.hpp"
#include "2iren/rhi/resources/swapchain.hpp"
#include "render_thread.hpp"
#include "2iren/rhi/resources/query.hpp"

namespace siren {

/**
 * @brief Encapsulates a mapped buffer pointer. This is used in streamed @ref Buffer's.
 */
struct MappedBufferPtr {
    /** @brief The mapped pointer. */
    void* ptr = nullptr;
    /** @brief The size of the buffer. */
    usize size = 0;
};

/**
 * @brief Information needed by the OpenGL backend for @ref Buffer's
 */
struct GlBufferDetails {
    /** @brief The descriptor of the @ref Buffer. */
    BufferDescriptor descriptor;
    /** @brief A mapped region of storage. Used iff the buffer type is BufferUsage::Stream. */
    MappedBufferPtr buffer_ptr;
};

/**
 * @brief Information needed by the OpenGL backend for @ref Image's
 */
struct GlImageDetails {
    /** @brief The descriptor of the @ref Image. */
    ImageDescriptor descriptor;
};

/**
 * @brief Information needed by the OpenGL backend for @ref Sampler's
 */
struct GlSamplerDetails {
    /** @brief The descriptor of the @ref Sampler. */
    SamplerDescriptor descriptor;
};

/**
 * @brief Information needed by the OpenGL backend for @ref Shader's
 */
struct GlShaderDetails {
    /** @brief The descriptor of the @ref Shader. */
    ShaderDescriptor descriptor;
    /** @brief The uniforms of the shader cached. */
    std::unordered_map<std::string, GLint> uniform_cache;
};

/**
 * @brief Information needed by the OpenGL backend for @ref GraphicPipeline's
 */
struct GlGraphicsPipelineDetails {
    /** @brief The descriptor of the @ref GraphicsPipeline. */
    GraphicsPipelineDescriptor descriptor;
};

struct GlSwapchainDetails {
    /** @brief The descriptor of the @ref Swapchain. */
    SwapchainDescriptor descriptor;
    /** @brief Raw handle to the window this @ref Framebuffer is associated with. */
    GLFWwindow* native_handle;
    struct Target {
        RenderTarget render_target;
        Image image;
    };
    /** @brief An @ref RenderTarget. Since OpenGL abstracts the swapchain away, we just render to a mock this. */
    std::optional<Target> target; // assume is always set, used over unique ptr
};

struct GlQueryDetails {
    /** @brief The descriptor of the @ref Query. */
    QueryDescriptor descriptor;
};

/**
 * @class FramebufferCache
 * @brief Used to cache and retrieve FBOs for the OpenGL backend. Since FBOs are vk/gl specific,
 * they do not exist in sirens main API.
 * Instead, we use render targets, which consist of images. However, OpenGL requires rendering to an FBO,
 * so we use this to create/fetch FBOs based on images.
 *
 * @todo @note Cached framebuffers are currently never cleaned up. do this homie
 */
class FramebufferCache {
public:
    explicit FramebufferCache(const RenderResourceTable<GLuint, Image, GlImageDetails>& image_table) :
        m_image_table{image_table} {};

    [[nodiscard]] auto get_create_for(const RenderTarget& target) -> GLuint;

private:
    struct Key {
        std::vector<ImageHandle> colors;
        ImageHandle depth_stencil; // set to NullHandle if not present
        auto operator==(const Key& key) const -> bool = default;
    };

    struct Hasher {
        auto operator()(const Key& key) const -> usize;
    };

    [[nodiscard]] auto create_framebuffer(const RenderTarget& target) -> GLuint;
    std::unordered_map<Key, GLuint, Hasher> m_cache{};
    const RenderResourceTable<GLuint, Image, GlImageDetails>& m_image_table;
};

/**
 * @struct RenderResourceState
 * @brief Encapsulates all @ref RenderResource state for the OpenGL backend.
 */
struct RenderResourceState {
    /** @brief Buffer handle storage. */
    RenderResourceTable<GLuint, Buffer, GlBufferDetails> buffer_table;
    /** @brief Image handle storage. */
    RenderResourceTable<GLuint, Image, GlImageDetails> image_table;
    /** @brief Sampler handle storage. */
    RenderResourceTable<GLuint, Sampler, GlSamplerDetails> sampler_table;
    /** @brief Shader handle storage. */
    RenderResourceTable<GLuint, Shader, GlShaderDetails> shader_table;
    /**
     * @brief GraphicsPipeline handle storage.
     * @note The GLuint stored here is not of the Pipeline, but rather the vertex array.
     * This is because OpenGL has no notion of a Pipeline, but we use a VA in the pipeline.
     */
    RenderResourceTable<GLuint, GraphicsPipeline, GlGraphicsPipelineDetails> graphics_pipeline_table;
    /**
     * @brief Swapchain handle storage.
     * @note GL doesn't have an exposed concept of a swapchain, so we store a meaningless void* here.
     */
    RenderResourceTable<void*, Swapchain, GlSwapchainDetails> swapchain_table;
    /** @brief Query handle storage. */
    RenderResourceTable<GLuint, Query, GlQueryDetails> query_table;
    /** @brief Manages fetching cached OpenGL framebuffers based on images. */
    mutable FramebufferCache framebuffer_cache{image_table};
};

class GlDevice final : public Device {
public:
    explicit GlDevice(GLFWwindow* window);
    ~GlDevice() override = default;

    auto wait_until_idle() const noexcept -> void override;

    [[nodiscard]] auto create_buffer(const BufferDescriptor& descriptor) -> Buffer override;
    auto destroy_buffer(BufferHandle handle) -> void override;

    [[nodiscard]] auto create_image(const ImageDescriptor& descriptor) -> Image override;
    auto destroy_image(ImageHandle handle) -> void override;

    [[nodiscard]] auto create_sampler(const SamplerDescriptor& descriptor) -> Sampler override;
    auto destroy_sampler(SamplerHandle handle) -> void override;

    [[nodiscard]] auto create_shader(const ShaderDescriptor& descriptor) -> Shader override;
    auto destroy_shader(ShaderHandle handle) -> void override;

    [[nodiscard]] auto create_swapchain(const SwapchainDescriptor& descriptor) -> Swapchain override;
    auto destroy_swapchain(SwapchainHandle handle) -> void override;

    [[nodiscard]] auto create_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
        -> GraphicsPipeline override;
    auto destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void override;

    [[nodiscard]] auto create_query(const QueryDescriptor& descriptor) -> Query override;
    auto destroy_query(QueryHandle handle) -> void override;


    auto flush_delete_queue() -> void override;

    [[nodiscard]] auto record_resource_commands() const -> ResourceCommandRecorder override;
    [[nodiscard]] auto record_render_commands() const -> RenderCommandRecorder override;
    auto submit(ResourceCommandBuffer&& command_buffer) const -> void override;
    auto submit(RenderCommandBuffer&& command_buffer) const -> void override;

    [[nodiscard]] auto buffer_descriptor(BufferHandle handle) const -> const BufferDescriptor& override;
    [[nodiscard]] auto image_descriptor(ImageHandle handle) const -> const ImageDescriptor& override;
    [[nodiscard]] auto sampler_descriptor(SamplerHandle handle) const -> const SamplerDescriptor& override;
    [[nodiscard]] auto shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& override;
    [[nodiscard]] auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& override;
    [[nodiscard]] auto swapchain_descriptor(SwapchainHandle handle) const -> const SwapchainDescriptor& override;
    [[nodiscard]] auto query_descriptor(QueryHandle handle) const -> const QueryDescriptor& override;

    auto query(QueryHandle handle) const -> u64 override;
    [[nodiscard]] auto acquire_next_swapchain_target(SwapchainHandle handle) const -> ImageHandle override;
    auto present(SwapchainHandle handle, OverlayFunction&& overlay = nullptr) const -> void override;
    auto blit(ImageHandle source, ImageHandle destination) const -> void override;

    [[nodiscard]] auto limits() const -> const Limits& override;
    [[nodiscard]] auto statistics() const -> Statistics override;

private:
    /**
     * @brief Enum listing all OpenGL GPU objects used.
     * @note We do not define a global enum for this, as different backends
     * may have a different way of doing things and not use the same GPU
     * objects.
     */
    enum class ResourceType {
        Buffer,
        Image,
        Sampler,
        Framebuffer,
        Shader,
        GraphicsPipeline,
        Query,
    };

    /** @brief The main worker thread for all rendering work. */
    RenderThread m_render_thread;

    /** @brief Describes a Delete that has been requested of a GPU object. */
    struct DeleteRequest {
        /** @brief The native OpenGL object handle. */
        GLuint handle;
        /** @brief The resource type of the object to be deleted. */
        ResourceType type;
    };

    /** @brief The state of @ref RenderResource's. */
    RenderResourceState m_state;

    /** @brief All objects queued for cleanup. */
    std::vector<DeleteRequest> m_delete_queue;

    /** @brief Copy of the statistics. */
    mutable Mutex<Statistics> m_statistics{};

    /** @brief Cached limits which are fetched once on startup. */
    Limits m_limits;
};

} // namespace siren
