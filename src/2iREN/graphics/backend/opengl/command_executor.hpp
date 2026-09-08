#pragma once

#include <glad/gl.h>

#include "2iREN/graphics/command_executor.hpp"
#include "2iREN/graphics/statistics.hpp"

namespace siren {

struct RenderResourceState;

/**
 * @brief Struct for the OpenGL backend tracking any
 * state by OpenGL needed for the @ref Executor.
 */
struct TrackedState {
    GLuint active_vao                      = 0;
    GraphicsPipelineHandle active_pipeline = NullHandle;
    BindIndexBuffer active_ibo             = {
        .index_buffer = BufferHandle::invalid(),
        .index_format = IndexFormat::UInt32
    };
};

/// @brief The OpenGL specific @ref CommandExecutor.
class OpenGLCommandExecutor final : public CommandExecutor {
public:
    explicit OpenGLCommandExecutor(const RenderResourceState& state);

    auto execute(RenderPass&& pass) -> void override;

    [[nodiscard]]
    auto statistics() const -> const Statistics& override;

private:
    const RenderResourceState& m_state;
    mutable TrackedState m_tracked_state;
    mutable Statistics m_statistics{};

    /** @brief Handles @ref BindGraphicsPipeline. */
    auto bind_graphics_pipeline(const BindGraphicsPipeline& bind) const -> void;
    /** @brief Handles @ref SetViewport. */
    auto set_viewport(const SetViewport& set_viewport, const RenderTarget& target) const -> void;

    /** @brief Handles @ref BindVertexBuffer. */
    auto bind_vertex_buffer(const BindVertexBuffer& bind_vertex_buffer) const -> void;
    /** @brief Handles @ref BindIndexBuffer. */
    auto bind_index_buffer(const BindIndexBuffer& bind_index_buffer) const -> void;
    /** @brief Handles @ref BindUniformBuffer. */
    auto bind_uniform_buffer(const BindUniformBuffer& bind_uniform_buffer) const -> void;
    /** @brief Handles @ref BindUniformBufferRange. */
    auto bind_uniform_buffer_range(const BindUniformBufferRange& bind_uniform_buffer_range) const
        -> void;
    /** @brief Handles @ref BindShaderStorageBuffer. */
    auto bind_shader_storage_buffer(const BindShaderStorageBuffer& bind_shader_storage_buffer) const
        -> void;

    /** @brief Handles @ref BindSampledImage. */
    auto bind_sampled_image(const BindSampledImage& bind_sampled_image) const -> void;
    /** @brief Handles @ref BindStorageImage. */
    auto bind_storage_image(const BindStorageImage& bind_storage_image) const -> void;
    /** @brief Handles @ref BeginQuery. */
    auto begin_query(const BeginQuery& begin_query) const -> void;
    /** @brief Handles @ref EndQuery. */
    auto end_query(const EndQuery& end_query) const -> void;
    /** @brief Handles @ref DrawArrays. */
    auto draw_arrays(const DrawArrays& draw_arrays) const -> void;
    /** @brief Handles @ref DrawIndexed. */
    auto draw_indexed(const DrawIndexed& draw_indexed) const -> void;
};

} // namespace siren
