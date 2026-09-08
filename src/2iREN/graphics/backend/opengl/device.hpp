#pragma once

#include <GLFW/glfw3.h>

#include <glad/gl.h>

#include <unordered_map>

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/utility/byte_buffer.hpp"

namespace siren {

/// @brief Encapsulates a mapped buffer pointer. This is used in streamed @ref Buffer's.
struct MappedBufferPtr {
    /// @brief The mapped pointer.
    void* ptr = nullptr;
    /// @brief The size of the buffer.
    usize size = 0;
};

/// @brief Information needed by the OpenGL backend for @ref Buffer's
struct GlBufferDetails {
    BufferDescriptor descriptor;
    /// @brief A mapped region of storage. Used iff the buffer type is BufferUsage::Stream.
    MappedBufferPtr buffer_ptr;
};

/// @brief Information needed by the OpenGL backend for @ref Image's
struct GlImageDetails {
    ImageDescriptor descriptor;
};

/// @brief Information needed by the OpenGL backend for @ref Sampler's
struct GlSamplerDetails {
    SamplerDescriptor descriptor;
};

/// @brief Information needed by the OpenGL backend for @ref Shader's
struct GlShaderDetails {
    ShaderDescriptor descriptor;
    /// @brief The uniforms of the shader cached.
    std::unordered_map<std::string, GLint> uniform_cache;
};

/// @brief Information needed by the OpenGL backend for @ref GraphicPipeline's
struct GlGraphicsPipelineDetails {
    GraphicsPipelineDescriptor descriptor;
};

struct GlSwapchainDetails {
    SwapchainDescriptor descriptor;
    GLFWwindow* native_handle;
    struct Target {
        RenderTarget render_target;
        Image image;
    };
    /// @brief An @ref RenderTarget. Since OpenGL abstracts the swapchain away, we just render to a
    /// mock this.
    std::optional<Target> target; // assume is always set, used over unique ptr
};

struct GlQueryDetails {
    QueryDescriptor descriptor;
};

/// @class FramebufferCache
/// @brief Used to cache and retrieve FBOs for the OpenGL backend. Since FBOs are vk/gl specific,
/// they do not exist in sirens main API.
/// Instead, we use render targets, which consist of images. However, OpenGL requires rendering to
/// an FBO, so we use this to create/fetch FBOs based on images.
///
/// @todo @note Cached framebuffers are currently never cleaned up. do this homie
class FramebufferCache {
public:
    explicit FramebufferCache(
        const RenderResourceTable<GLuint, Image, GlImageDetails>& image_table
    ) : m_image_table{image_table} { };

    [[nodiscard]]
    auto get_create_for(const RenderTarget& target) -> GLuint;

private:
    struct Key {
        std::vector<ImageHandle> colors;
        ImageHandle depth_stencil; // set to NullHandle if not present
        auto operator==(const Key& key) const -> bool = default;
    };

    struct Hasher {
        auto operator()(const Key& key) const -> usize;
    };

    [[nodiscard]]
    auto create_framebuffer(const RenderTarget& target) const -> GLuint;
    std::unordered_map<Key, GLuint, Hasher> m_cache{};
    const RenderResourceTable<GLuint, Image, GlImageDetails>& m_image_table;
};

/// @struct RenderResourceState
/// @brief Encapsulates all @ref RenderResource state for the OpenGL backend.
struct RenderResourceState {
    /// @brief Buffer handle storage.
    RenderResourceTable<GLuint, Buffer, GlBufferDetails> buffer_table;
    /// @brief Image handle storage.
    RenderResourceTable<GLuint, Image, GlImageDetails> image_table;
    /// @brief Sampler handle storage.
    RenderResourceTable<GLuint, Sampler, GlSamplerDetails> sampler_table;
    /// @brief Shader handle storage.
    RenderResourceTable<GLuint, Shader, GlShaderDetails> shader_table;
    /// @brief GraphicsPipeline handle storage.
    /// @note The GLuint stored here is not of the Pipeline, but rather the vertex array.
    /// This is because OpenGL has no notion of a Pipeline, but we use a VA in the pipeline.
    RenderResourceTable<GLuint, GraphicsPipeline, GlGraphicsPipelineDetails>
        graphics_pipeline_table;
    /// @brief Swapchain handle storage.
    /// @note GL doesn't have an exposed concept of a swapchain, so we store a meaningless void*
    /// here.
    RenderResourceTable<void*, Swapchain, GlSwapchainDetails> swapchain_table;
    /// @brief Query handle storage.
    RenderResourceTable<GLuint, Query, GlQueryDetails> query_table;
    /// @brief Manages fetching cached OpenGL framebuffers based on images.
    mutable FramebufferCache framebuffer_cache{image_table};
};

class OpenGLDevice final : public Device {
public:
    explicit OpenGLDevice();
    ~OpenGLDevice() override;

    [[nodiscard]]
    auto make_buffer(const BufferDescriptor& descriptor, std::optional<ByteBufferView> initial)
        -> Buffer override;

    [[nodiscard]]
    auto make_image(const ImageDescriptor& descriptor) -> Image override;

    [[nodiscard]]
    auto make_sampler(const SamplerDescriptor& descriptor) -> Sampler override;

    [[nodiscard]]
    auto make_shader(const ShaderDescriptor& descriptor) -> Shader override;

    [[nodiscard]]
    auto make_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
        -> GraphicsPipeline override;

    [[nodiscard]]
    auto make_swapchain(const Window& window, const SwapchainDescriptor& descriptor)
        -> Swapchain override;

    [[nodiscard]]
    auto make_query(const QueryDescriptor& descriptor) -> Query override;

    auto destroy_buffer(BufferHandle handle) -> void override;

    auto destroy_image(ImageHandle handle) -> void override;

    auto destroy_sampler(SamplerHandle handle) -> void override;

    auto destroy_shader(ShaderHandle handle) -> void override;

    auto destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void override;

    auto destroy_swapchain(SwapchainHandle handle) -> void override;

    auto destroy_query(QueryHandle handle) -> void override;

    [[nodiscard]]
    auto buffer_descriptor(BufferHandle handle) const -> const BufferDescriptor& override;

    [[nodiscard]]
    auto image_descriptor(ImageHandle handle) const -> const ImageDescriptor& override;

    [[nodiscard]]
    auto sampler_descriptor(SamplerHandle handle) const -> const SamplerDescriptor& override;

    [[nodiscard]]
    auto shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& override;

    [[nodiscard]]
    auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& override;

    [[nodiscard]]
    auto swapchain_descriptor(SwapchainHandle handle) const -> const SwapchainDescriptor& override;

    [[nodiscard]]
    auto query_descriptor(QueryHandle handle) const -> const QueryDescriptor& override;

    [[nodiscard]]
    auto render_pass_recorder(const RenderPassDescriptor& descriptor) const noexcept
        -> RenderPassRecorder override;

    auto submit(RenderPass&& pass) const -> void override;

    auto upload_to_image(ImageHandle image, ByteBufferView data, usize layer) const
        -> void override;

    auto upload_to_buffer(BufferHandle buffer, ByteBufferView data, usize offset) const
        -> void override;

    auto clear_image(ImageHandle image, ClearValue clearvalue) const -> void override;

    auto blit_to_image(ImageHandle source, ImageHandle destination) const -> void override;

    auto read_image(ImageHandle image) const -> std::vector<u8> override;

    auto present(SwapchainHandle handle, OverlayFunction&& overlay = nullptr) const
        -> void override;

    auto query_result(QueryHandle handle) const -> u64 override;

    auto begin_conditional_render(QueryHandle query) const -> void override;

    auto end_conditional_render() const -> void override;

    [[nodiscard]]
    auto limits() const -> const Limits& override;

    [[nodiscard]]
    auto statistics() const -> Statistics override;

    [[nodiscard]]
    auto acquire_next_swapchain_target(SwapchainHandle handle) const -> ImageHandle override;

    auto wait_idle() const noexcept -> void override;

private:
    /// @brief The state of @ref RenderResource's.
    RenderResourceState m_state;

    /// @brief Copy of the statistics.
    mutable Statistics m_statistics{};

    /// @brief Cached limits which are fetched once on startup.
    Limits m_limits{};
};

} // namespace siren
