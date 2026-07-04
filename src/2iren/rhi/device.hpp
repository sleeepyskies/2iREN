#pragma once

#include "render_command.hpp"
#include "resource_command.hpp"
#include "resources/fwd.hpp"

namespace siren {

/**
 * @brief Defines the hardware limits of the current backend.
 * @todo impl this
 */
struct Limits {
    u32 max_buffer_slots;
};

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

    /** @brief Creates and returns a new @ref Framebuffer given a @ref FramebufferDescriptor. */
    [[nodiscard]] virtual auto create_framebuffer(const FramebufferDescriptor& descriptor) -> Framebuffer = 0;
    /** @brief Queues the given @ref Framebuffer for deletion. */
    virtual auto destroy_framebuffer(FramebufferHandle handle) -> void = 0;

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
    /** @brief Returns the @ref FramebufferDescriptor associated with this handle. */
    [[nodiscard]] virtual auto framebuffer_descriptor(FramebufferHandle handle) const
        -> const FramebufferDescriptor& = 0;
    /** @brief Returns the @ref ShaderDescriptor associated with this handle. */
    [[nodiscard]] virtual auto shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& = 0;
    /** @brief Returns the @ref GraphicsPipelineDescriptor associated with this handle. */
    [[nodiscard]] virtual auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& = 0;
    /** @brief Returns the @ref ShaderDescriptor associated with this handle. */
    [[nodiscard]] virtual auto swapchain_descriptor(SwapchainHandle handle) const -> const SwapchainDescriptor& = 0;

    /** @brief Returns the next @ref Image target managed by this framebuffer to render to. */
    [[nodiscard]] virtual auto acquire_next_swapchain_target(SwapchainHandle handle) const -> FramebufferHandle = 0;

    /** @brief Presents the back buffer of the given swapchain to the screen. */
    virtual auto present(SwapchainHandle handle) const -> void = 0;

    /** @brief Returns all the attachments belonging to the given @ref Framebuffer.. */
    virtual auto framebuffer_attachments(FramebufferHandle handle) const -> const FramebufferAttachments& = 0;

    /** @brief Returns the hardware limits of the current backend. */
    [[nodiscard]] virtual auto limits() const -> Limits = 0;
};

} // namespace siren
