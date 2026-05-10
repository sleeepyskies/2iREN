#pragma once

#include "2iren/command_executor.hpp"
#include "device.hpp"


namespace siren {

/**
 * @brief Struct for the OpenGL backend tracking any
 * state by OpenGL needed for the @ref Executor.
 */
struct TrackedState {
    GLuint active_vao                      = 0;
    GraphicsPipelineHandle active_pipeline = NullHandle;
    BindIndexBuffer active_ibo             = {
        .index_buffer = BufferHandle::invalid(),
        .index_format = IndexFormat::Uint32
    };
};

/**
 * @class GlCommandExecutor
 * @brief The OpenGL specific @ref CommandExecutor.
 */
class GlCommandExecutor final : public CommandExecutor {
public:
    explicit GlCommandExecutor(const RenderResourceState& state);
    ~GlCommandExecutor() override = default;

    /**
     * @brief Executes the provided @ref ResourceCommand's
     * @param resource_command_pacakge The commands to execute.
     * @note This function should only be called from within a RenderThread::spawn lambda!!!!
     */
    auto execute(ResourceCommandBuffer&& resource_command_pacakge) -> void override;

    /**
     * @brief Executes the provided @ref RenderCommands's
     * @param render_command_package The commands to execute.
     * @note This function should only be called from within a RenderThread::spawn lambda!!!!
     */
    auto execute(RenderCommandBuffer&& render_command_package) -> void override;

private:
    const RenderResourceState& m_state;
    mutable TrackedState m_tracked_state;

    /** @brief Handles @ref UploadImage. */
    auto execute_image_upload(const UploadImage& cmd, std::span<const u8> data_slice) const -> void;
    /** @brief Handles @ref UploadBuffer. */
    auto execute_buffer_upload(const UploadBuffer& cmd, std::span<const u8> data_slice) const -> void;

    /** @brief Executes a single @ref RenderPass. */
    auto execute_pass(
        const RenderPassDescriptor& descriptor,
        std::span<const RenderCommand> commands
    ) const -> void;

    /** @brief Handles @ref BindGraphicsPipeline. */
    auto bind_graphics_pipeline(const BindGraphicsPipeline& bind) const -> void;
    /** @brief Handles @ref SetViewport. */
    auto set_viewport(const SetViewport& set_viewport, FramebufferHandle fb_handle) const -> void;
    /** @brief Handles @ref BindVertexBuffer. */
    auto bind_vertex_buffer(const BindVertexBuffer& bind_vertex_buffer) const -> void;
    /** @brief Handles @ref BindIndexBuffer. */
    auto bind_index_buffer(const BindIndexBuffer& bind_index_buffer) const -> void;
    /** @brief Handles @ref BindUniformBuffer. */
    auto bind_uniform_buffer(const BindUniformBuffer& bind_uniform_buffer) const -> void;
    /** @brief Handles @ref DrawArrays. */
    auto draw_arrays(const DrawArrays& draw_arrays) const -> void;
    /** @brief Handles @ref DrawIndexed. */
    auto draw_indexed(const DrawIndexed& draw_indexed) const -> void;
};

} // namespace siren
