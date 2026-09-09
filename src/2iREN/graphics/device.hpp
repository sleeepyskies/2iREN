#pragma once

#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/limits.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/statistics.hpp"
#include "2iREN/utility/byte_buffer.hpp"

namespace siren {

class Window;

using ClearValue = std::variant<Rgba, u32>;

using OverlayFunction = std::function<void()>;

/// @brief The Device manages the lifetime of @ref RenderResource objects.
/// Furthermore, it is the primary entry point for all interactions with the GPU.
class Device {
public:
    virtual ~Device() = default;

    /// @brief Creates and returns a new @ref Buffer given a @ref BufferDescriptor.
    [[nodiscard]]
    virtual auto make_buffer(
        const BufferDescriptor& descriptor,
        std::optional<ByteBufferView> initial = std::nullopt
    ) -> Buffer = 0;

    /// @brief Creates and returns a new @ref Image given an @ref ImageDescriptor.
    [[nodiscard]]
    virtual auto make_image(const ImageDescriptor& descriptor) -> Image = 0;

    /// @brief Creates and returns a new @ref Image given an @ref ImageDescriptor.
    [[nodiscard]]
    virtual auto make_sampler(const SamplerDescriptor& descriptor) -> Sampler = 0;

    /// @brief Creates and returns a new @ref Shader given a @ref ShaderDescriptor.
    [[nodiscard]]
    virtual auto make_shader(const ShaderDescriptor& descriptor) -> Shader = 0;

    /// @brief Creates and returns a new @ref GraphicsPipeline given a @ref
    /// GraphicsPipelineDescriptor.
    [[nodiscard]]
    virtual auto make_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
        -> GraphicsPipeline = 0;

    /// @brief Creates and returns a new @ref Swapchain given a @ref SwapchainDescriptor.
    [[nodiscard]]
    virtual auto make_swapchain(const Window& window, const SwapchainDescriptor& descriptor)
        -> Swapchain = 0;

    /// @brief Creates and returns a new @ref Query given.
    [[nodiscard]]
    virtual auto make_query(const QueryDescriptor& descriptor) -> Query = 0;

    /// @brief Queues the given @ref Buffer for deletion.
    virtual auto destroy_buffer(BufferHandle handle) -> void = 0;

    /// @brief Queues the given @ref Image for deletion.
    virtual auto destroy_image(ImageHandle handle) -> void = 0;

    /// @brief Queues the given @ref ImageSampler for deletion.
    virtual auto destroy_sampler(SamplerHandle handle) -> void = 0;

    /// @brief Queues the given @ref Shader for deletion.
    virtual auto destroy_shader(ShaderHandle handle) -> void = 0;

    /// @brief Queues the given @ref GraphicsPipeline for deletion.
    virtual auto destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void = 0;

    /// @brief Queues the given @ref Swapchain for deletion.
    virtual auto destroy_swapchain(SwapchainHandle handle) -> void = 0;

    /// @brief Queues the given @ref Query for deletion.
    virtual auto destroy_query(QueryHandle handle) -> void = 0;

    /// @brief Returns the @ref BufferDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto buffer_descriptor(BufferHandle handle) const -> const BufferDescriptor& = 0;

    /// @brief Returns the @ref ImageDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto image_descriptor(ImageHandle handle) const -> const ImageDescriptor& = 0;

    /// @brief Returns the @ref SamplerDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto sampler_descriptor(SamplerHandle handle) const -> const SamplerDescriptor& = 0;

    /// @brief Returns the @ref ShaderDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& = 0;

    /// @brief Returns the @ref GraphicsPipelineDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& = 0;

    /// @brief Returns the @ref ShaderDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto swapchain_descriptor(SwapchainHandle handle) const
        -> const SwapchainDescriptor& = 0;

    /// @brief Returns the @ref QueryDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto query_descriptor(QueryHandle handle) const -> const QueryDescriptor& = 0;

    /// @brief Returns a recorder to record render commands into.
    [[nodiscard]]
    virtual auto render_pass_recorder(const RenderPassDescriptor& descriptor) const noexcept
        -> RenderPassRecorder = 0;

    /// @brief Submits a @ref RenderPass for execution.
    virtual auto submit(RenderPass&& pass) const -> void = 0;

    /// @brief Records and submits a render pass.
    template <typename Function>
        requires(std::is_invocable_v<Function, RenderPassRecorder&>)
    auto render_pass(RenderPassDescriptor&& descriptor, Function&& func) noexcept -> void {
        auto recorder = render_pass_recorder(descriptor);
        std::invoke(func, recorder);
        submit(recorder.finish());
    }

    /// @brief Uploads data to an image.
    virtual auto upload_to_image(ImageHandle image, ByteBufferView data, usize layer) const
        -> void = 0;

    /// @brief Uploads data to a buffer.
    virtual auto upload_to_buffer(BufferHandle buffer, ByteBufferView data, usize offset) const
        -> void = 0;

    /// @brief Clears all pixels of an image using the provided value.
    virtual auto clear_image(ImageHandle image, ClearValue clearvalue) const -> void = 0;

    /// @brief Copies the content of an @ref Image to another @ref Image. @note Assumes source
    /// and destination have the same size.
    virtual auto blit_to_image(ImageHandle source, ImageHandle destination) const -> void = 0;

    /// @brief Reads the image data into a buffer and returns it. @warning May stall the thread
    /// until task is complete.
    [[nodiscard]]
    virtual auto read_image(ImageHandle image) const -> std::vector<u8> = 0;

    /// @brief Presents the back buffer of the given swapchain to the screen and also executes a
    /// custom overlay function.
    virtual auto present(SwapchainHandle handle, OverlayFunction&& overlay = nullptr) const
        -> void = 0;

    /// @brief Returns the next @ref Image target managed by this framebuffer to render to.
    [[nodiscard]] virtual auto acquire_next_swapchain_target(SwapchainHandle handle) const
        -> ImageHandle = 0;

    /// @brief Retrieves the information stored inside a query object. May be blocking on some
    /// implementations. The return value must be interpreted by the caller depending on the
    /// QueryKind.
    [[nodiscard]]
    virtual auto query_result(QueryHandle handle) const -> u64 = 0;

    /// @brief Checks if a query objects result is available.
    [[nodiscard]]
    virtual auto query_available(QueryHandle handle) const -> bool = 0;

    /// @brief Begins a conditionally rendered scope. Any draw calls between
    /// this and end_conditional_render may be omitted based on the query object.
    virtual auto begin_conditional_render(QueryHandle query) const -> void = 0;

    /// @brief Ends a conditionally rendered scope.
    virtual auto end_conditional_render() const -> void = 0;

    /// @brief Returns the hardware limits of the current backend.
    [[nodiscard]]
    virtual auto limits() const -> const Limits& = 0;

    /// @brief Returns the accumulated rendering statistics since the last time this function was
    /// called.
    [[nodiscard]]
    virtual auto statistics() const -> Statistics = 0;

    /// @brief Blocks until the device has finished all tasks.
    virtual auto wait_idle() const noexcept -> void = 0;
};
} // namespace siren
