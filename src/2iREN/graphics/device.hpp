#pragma once

#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/limits.hpp"
#include "2iREN/graphics/statistics.hpp"
#include "2iREN/utility/byte_buffer.hpp"

namespace siren {

class Window;

using ClearValue = std::variant<Rgba, u32>;

/// @brief The Device manages the lifetime of @ref RenderResource objects.
/// Furthermore, it is the primary entry point for all interactions with the
/// GPU.
class Device {
public:
    virtual ~Device() = default;

    /// @brief Creates and returns a new @ref Buffer given a @ref
    /// BufferDescriptor.
    [[nodiscard]]
    virtual auto make_buffer(
        const BufferDescriptor&       descriptor,
        std::optional<ByteBufferView> initial = std::nullopt
    ) -> Buffer = 0;

    /// @brief Creates and returns a new @ref Image given an @ref
    /// ImageDescriptor.
    [[nodiscard]]
    virtual auto make_image(
        const ImageDescriptor&        descriptor,
        std::optional<ByteBufferView> initial = std::nullopt
    ) -> Image = 0;

    /// @brief Creates and returns a new @ref Image given an @ref
    /// ImageDescriptor.
    [[nodiscard]]
    virtual auto make_sampler(const SamplerDescriptor& descriptor) -> Sampler = 0;

    /// @brief Creates and returns a new @ref Shader given a @ref
    /// ShaderDescriptor.
    [[nodiscard]]
    virtual auto make_shader(const ShaderDescriptor& descriptor) -> Shader = 0;

    /// @brief Creates and returns a new @ref GraphicsPipeline given a @ref
    /// GraphicsPipelineDescriptor.
    [[nodiscard]]
    virtual auto make_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
        -> GraphicsPipeline = 0;

    /// @brief Creates and returns a new @ref Swapchain given a @ref
    /// SwapchainDescriptor.
    [[nodiscard]]
    virtual auto make_swapchain(const Window& window, const SwapchainDescriptor& descriptor)
        -> Swapchain = 0;

    /// @brief Creates and returns a new @ref Query.
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

    /// @brief Returns the @ref GraphicsPipelineDescriptor associated with this
    /// handle.
    [[nodiscard]]
    virtual auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& = 0;

    /// @brief Returns the @ref ShaderDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto swapchain_descriptor(SwapchainHandle handle) const
        -> const SwapchainDescriptor& = 0;

    /// @brief Returns information about the swapchain.
    [[nodiscard]]
    virtual auto swapchain_info(SwapchainHandle handle) const -> SwapchainInfo = 0;

    /// @brief Returns the @ref QueryDescriptor associated with this handle.
    [[nodiscard]]
    virtual auto query_descriptor(QueryHandle handle) const -> const QueryDescriptor& = 0;

    /// @brief Returns a new command recorder.
    [[nodiscard]]
    virtual auto make_command_recorder() const noexcept -> CommandRecorder {
        return CommandRecorder{this};
    };

    /// @brief Submits a list of 2iREN commands to be translated for the
    /// backend and for exection on the GPU.
    virtual auto submit(CommandList&& cmds) -> void = 0;

    /// @brief Submits a list of 2iREN commands to be translated for the
    /// backend and for exection on the GPU.
    auto submit(CommandRecorder&& cmds) -> void {
        submit(std::move(cmds).finish());
    }

    /// @brief Presents the back buffer of the given swapchain to the screen.
    /// @note Does not guarantee that work using the backbuffer is completed
    /// before presenting.
    virtual auto present(SwapchainHandle handle) -> void = 0;

    /// @brief Presents the back buffer of the given swapchain to the screen.
    /// Furthremore, the backbuffer will only be presented once the provided
    /// work has been completed by the GPU.
    virtual auto present(SwapchainHandle handle, CommandList&& cmds) -> void = 0;

    /// @brief Returns the next @ref Image target managed by this framebuffer to
    /// render to.
    [[nodiscard]] virtual auto acquire_next_swapchain_image(SwapchainHandle handle)
        -> ImageHandle = 0;

    /// @brief Retrieves the information stored inside a query object. May be
    /// blocking on some implementations. The return value must be interpreted
    /// by the caller depending on the QueryKind.
    [[nodiscard]]
    virtual auto query_result(QueryHandle handle) const -> u64 = 0;

    /// @brief Checks if a query objects result is available.
    [[nodiscard]]
    virtual auto query_available(QueryHandle handle) const -> bool = 0;

    /// @brief Begins a conditionally rendered scope. Any draw calls between
    /// this and end_conditional_render may be omitted based on the query
    /// object.
    virtual auto begin_conditional_render(QueryHandle query) const -> void = 0;

    /// @brief Ends a conditionally rendered scope.
    virtual auto end_conditional_render() const -> void = 0;

    /// @brief Blocks until the device has finished all tasks.
    virtual auto wait_idle() const noexcept -> void = 0;

    /// @brief Returns the hardware limits of the current backend.
    [[nodiscard]]
    auto limits() const noexcept -> const Limits& {
        return m_limits;
    }

    /// @brief Returns the accumulated rendering statistics since the last time
    /// this function was called.
    [[nodiscard]]
    auto statistics() const noexcept -> Statistics {
        return std::exchange(m_statistics, {});
    }

    /// @brief Returns the active backend kind.
    [[nodiscard]]
    auto backend() const noexcept -> Backend {
        return m_backend;
    }

protected:
    Device(Backend backend) : m_backend(backend) { }

    mutable Limits     m_limits     = {};
    mutable Statistics m_statistics = {};
    Backend            m_backend;
};
} // namespace siren
