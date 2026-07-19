#pragma once

#include "render_command.hpp"
#include "resource_command.hpp"
#include "resources/fwd.hpp"

namespace siren {

/**
 * @struct Statistics
 * @brief Collects statistics regarding the amount and types of operations
 * performed by the rendering backend.
 */
struct Statistics {
    /** @brief The number of @ref BindGraphicsPipeline commands. */
    u32 count_bind_graphics_pipeline;
    /** @brief The number of @ref SetViewport commands. */
    u32 count_set_viewport;
    /** @brief The number of @ref BindVertexBuffer commands. */
    u32 count_bind_vertex_buffer;
    /** @brief The number of @ref BindIndexBuffer commands. */
    u32 count_bind_index_buffer;
    /** @brief The number of @ref BindUniformBuffer commands. */
    u32 count_bind_uniform_buffer;
    /** @brief The number of @ref BindSampledImage commands. */
    u32 count_bind_sampled_image;
    /** @brief The number of @ref BindStorageImage commands. */
    u32 count_bind_storage_image;
    /** @brief The number of @ref DrawArrays commands. */
    u32 count_draw_arrays;
    /** @brief The number of @ref DrawIndexed commands. */
    u32 count_draw_indexed;
    /** @brief The number of @ref UploadBuffer commands. */
    u32 count_upload_buffer;
    /** @brief The number of @ref UploadImage commands. */
    u32 count_upload_image;

    auto operator+=(const Statistics& rhs) noexcept -> Statistics& {
        count_bind_graphics_pipeline += rhs.count_bind_graphics_pipeline;
        count_set_viewport += rhs.count_set_viewport;
        count_bind_vertex_buffer += rhs.count_bind_vertex_buffer;
        count_bind_index_buffer += rhs.count_bind_index_buffer;
        count_bind_uniform_buffer += rhs.count_bind_uniform_buffer;
        count_bind_sampled_image += rhs.count_bind_sampled_image;
        count_bind_storage_image += rhs.count_bind_storage_image;
        count_draw_arrays += rhs.count_draw_arrays;
        count_draw_indexed += rhs.count_draw_indexed;
        count_upload_buffer += rhs.count_upload_buffer;
        count_upload_image += rhs.count_upload_image;
        return *this;
    }

    friend auto operator+(Statistics lhs, const Statistics& rhs) noexcept -> Statistics {
        lhs += rhs;
        return lhs;
    }
};

/**
 * @brief Defines the hardware limits of the current backend.
 * @todo impl this
 */
struct Limits {
    u32 max_buffer_slots;
};

using OverlayFunction = std::function<void()>;

/**
 * @brief The Device manages the lifetime of @ref RenderResource objects.
 * Furthermore, it is the primary entry point for all interactions with the GPU.
 * @note Many Device operations are performed asynchronously. In the case of
 * RenderResource creation, the siren abstraction is retuned immediately, but
 * the GPU object may not exist yet. However, the @ref RenderThread executes tasks
 * sequentially, and thus any requested operations on a GPU object should be fine.
 */
class Device {
public:
    virtual ~Device() = default;

    /** @brief Blocks the calling thread until there is no GPU work left to be done. */
    virtual auto wait_until_idle() const noexcept -> void = 0;

    /** @brief Creates and returns a new @ref Buffer given a @ref BufferDescriptor. */
    [[nodiscard]] virtual auto create_buffer(const BufferDescriptor& descriptor) -> Buffer = 0;
    /** @brief Queues the given @ref Buffer for deletion. */
    virtual auto destroy_buffer(BufferHandle handle) -> void = 0;

    /** @brief Creates and returns a new @ref Image given an @ref ImageDescriptor. */
    [[nodiscard]] virtual auto create_image(const ImageDescriptor& descriptor) -> Image = 0;
    /** @brief Queues the given @ref Image for deletion. */
    virtual auto destroy_image(ImageHandle handle) -> void = 0;

    /** @brief Creates and returns a new @ref Image given an @ref ImageDescriptor. */
    [[nodiscard]] virtual auto create_sampler(const SamplerDescriptor& descriptor) -> Sampler = 0;
    /** @brief Queues the given @ref ImageSampler for deletion. */
    virtual auto destroy_sampler(SamplerHandle handle) -> void = 0;

    /** @brief Creates and returns a new @ref Shader given a @ref ShaderDescriptor. */
    [[nodiscard]] virtual auto create_shader(const ShaderDescriptor& descriptor) -> Shader = 0;
    /** @brief Queues the given @ref Shader for deletion. */
    virtual auto destroy_shader(ShaderHandle handle) -> void = 0;

    /** @brief Creates and returns a new @ref GraphicsPipeline given a @ref GraphicsPipelineDescriptor. */
    [[nodiscard]] virtual auto create_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
        -> GraphicsPipeline = 0;
    /** @brief Queues the given @ref GraphicsPipeline for deletion. */
    virtual auto destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void = 0;

    /** @brief Creates and returns a new @ref Swapchain given a @ref SwapchainDescriptor. */
    [[nodiscard]] virtual auto create_swapchain(const SwapchainDescriptor& descriptor) -> Swapchain = 0;
    /** @brief Queues the given @ref Swapchain for deletion. */
    virtual auto destroy_swapchain(SwapchainHandle handle) -> void = 0;

    /** @brief Flushes the delete queue of the device. Must be called once a frame. */
    virtual auto flush_delete_queue() -> void = 0;

    /** @brief Creates and returns a new @ref ResourceCommandBuffer. */
    [[nodiscard]] virtual auto record_resource_commands() const -> ResourceCommandRecorder = 0;
    /** @brief Creates and returns a new @ref RenderCommandBuffer. Provides an API for execution tasks. */
    [[nodiscard]] virtual auto record_render_commands() const -> RenderCommandRecorder = 0;
    /** @brief Submits a @ref ResourceCommandPacakge for execution. */
    virtual auto submit(ResourceCommandBuffer&& command_buffer) const -> void = 0;
    /** @brief Submits a @ref RenderCommandPackage for execution. */
    virtual auto submit(RenderCommandBuffer&& command_buffer) const -> void = 0;

    /** @brief QOL function that provides a scoped way to record render commands and automatically submit them. */
    template <typename Function>
        requires(std::is_invocable_v<Function, RenderCommandRecorder&>)
    auto render_submit(Function&& func) const noexcept -> void {
        auto cmds = record_render_commands();
        std::invoke(func, cmds);
        submit(cmds.finish());
    }

    /** @brief QOL function that provides a scoped way to record resource commands and automatically submit them. */
    template <typename Function>
        requires(std::is_invocable_v<Function, ResourceCommandRecorder&>)
    auto resource_submit(Function&& func) const noexcept -> void {
        auto cmds = record_resource_commands();
        std::invoke(func, cmds);
        submit(cmds.finish());
    }

    /** @brief Returns the @ref BufferDescriptor associated with this handle. */
    [[nodiscard]] virtual auto buffer_descriptor(BufferHandle handle) const -> const BufferDescriptor& = 0;
    /** @brief Returns the @ref ImageDescriptor associated with this handle. */
    [[nodiscard]] virtual auto image_descriptor(ImageHandle handle) const -> const ImageDescriptor& = 0;
    /** @brief Returns the @ref SamplerDescriptor associated with this handle. */
    [[nodiscard]] virtual auto sampler_descriptor(SamplerHandle handle) const -> const SamplerDescriptor& = 0;
    /** @brief Returns the @ref ShaderDescriptor associated with this handle. */
    [[nodiscard]] virtual auto shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& = 0;
    /** @brief Returns the @ref GraphicsPipelineDescriptor associated with this handle. */
    [[nodiscard]] virtual auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& = 0;
    /** @brief Returns the @ref ShaderDescriptor associated with this handle. */
    [[nodiscard]] virtual auto swapchain_descriptor(SwapchainHandle handle) const -> const SwapchainDescriptor& = 0;

    /** @brief Returns the next @ref Image target managed by this framebuffer to render to. */
    [[nodiscard]] virtual auto acquire_next_swapchain_target(SwapchainHandle handle) const -> ImageHandle = 0;

    /// todo: i'm not super happy with this api, maybe a more formal overlay system would be better
    /** @brief Presents the back buffer of the given swapchain to the screen and also executes a custom overlay function. */
    virtual auto present(SwapchainHandle handle, OverlayFunction&& overlay = nullptr) const -> void = 0;

    /** @brief Copies the content of an @ref Image to another @ref Image. @note Assumes source and destination have the
     * same size. */
    virtual auto blit(ImageHandle source, ImageHandle destination) const -> void = 0;

    /** @brief Returns the hardware limits of the current backend. */
    [[nodiscard]] virtual auto limits() const -> Limits = 0;

    /** @brief Returns the accumulated rendering backend statistics since the last time this function was called. */
    [[nodiscard]] virtual auto statistics() const -> Statistics = 0;
};

} // namespace siren
