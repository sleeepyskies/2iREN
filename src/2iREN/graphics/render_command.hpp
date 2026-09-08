#pragma once

#include <unordered_map>

#include "2iREN/core/base.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/render_target.hpp"

namespace siren {
// todo: optimization here to use a packed blob vector. we serialize the commands basically,
// and make use of a CommandHeader indicating the size and type to interpret the next bytes as

enum class AccessKind {
    ReadOnly,
    WriteOnly,
    ReadWrite,
};

/**
 * @brief Identifies the type of operation recorded into the buffer.
 * Acts as a tag for a union.
 */
enum class RenderCommandType : u8 {
    BindGraphicsPipeline,
    SetViewport,

    BindVertexBuffer,
    BindIndexBuffer,
    BindUniformBuffer,
    BindUniformBufferRange,
    BindShaderStorageBuffer,

    BindSampledImage,
    BindStorageImage,

    BeginQuery,
    EndQuery,

    DrawArrays,
    DrawIndexed,
};

/**
 * @brief Indicates a @ref GraphicsPipeline bind. Sets all of its state.
 */
struct BindGraphicsPipeline {
    /** @brief The pipeline to bind. */
    GraphicsPipelineHandle pipeline_handle;
};

/**
 * @brief Sets the viewport transform (from NDC to target coords).
 */
struct SetViewport {
    /** @brief The offset in pixels from the left of the viewport. */
    u32 x;
    /** @brief The offset in pixels from the top of the viewport. */
    u32 y;
    /** @brief The width in pixels of the viewport. */
    u32 width;
    /** @brief The height in pixels of the viewport. */
    u32 height;
};

/**
 * @brief Binds a vertex buffer to a slot.
 */
struct BindVertexBuffer {
    /** @brief The buffer to bind. */
    BufferHandle vertex_buffer;
    /** @brief The slot to bind to. */
    u32 slot;
    /** @brief The offset into the vertex buffer. */
    u32 offset;
};

/**
 * @brief Binds an index buffer.
 */
struct BindIndexBuffer {
    /** @brief The buffer to bind. */
    BufferHandle index_buffer;
    /** @brief The format of the indices. */
    IndexFormat index_format;
};

/**
 * @brief Binds a uniform buffer.
 */
struct BindUniformBuffer {
    /** @brief The buffer to bind. */
    BufferHandle uniform_buffer;
    /** @brief The slot to bind to. */
    u32 slot;
};

/**
 * @brief Binds a subsection of a uniform buffer.
 */
struct BindUniformBufferRange {
    /** @brief The buffer to bind. */
    BufferHandle uniform_buffer;
    /** @brief The slot to bind to. */
    u32 slot;
    /** @brief The offset in bytes into the buffer to begin the binding range. */
    usize offset;
    /** @brief The size of the sub binding range. */
    usize size;
};

/**
 * @brief Binds a Shader Storage Buffer Object.
 */
struct BindShaderStorageBuffer {
    /** @brief The buffer to bind. */
    BufferHandle shader_storage_buffer;
    /** @brief The slot to bind to. */
    u32 slot;
};

/**
 * @brief Binds an @ref Image for sampled access. This uses filtering and mipmap sampling.
 * This also allows only for read access and uses texture coordinates instead of pixel coordinates.
 */
struct BindSampledImage {
    /** @brief The @ref Image to bind. */
    ImageHandle image;
    /** @brief The @ref Sampler to use. */
    SamplerHandle sampler;
    /** @brief The slot to bind to. */
    u32 slot;
};

/**
 * @brief Binds an @ref Image for direct pixel access. This applies no filtering or mip map
 * sampling. This also allows for read write access and uses raw pixel coordinates instead of
 * texture coordinates.
 */
struct BindStorageImage {
    /** @brief The image to bind. */
    ImageHandle image;
    /** @brief The slot to bind to. */
    u32 slot;
    /** @brief Specifies how the shader may access the @ref Image. */
    AccessKind access;
};

/**
 * @brief Begins recording into a @ref Query.
 */
struct BeginQuery {
    /** @brief The @ref Query to begin. */
    QueryHandle query;
};

/**
 * @brief Ends recording into a @ref Query.
 */
struct EndQuery {
    /** @brief The @ref Query to end. */
    QueryHandle query;
};

/**
 * @brief Performs a non indexed draw call.
 */
struct DrawArrays {
    /** @brief The start vertex to draw. */
    u32 start;
    /** @brief The amount of vertices to draw. */
    u32 count;
};

/**
 * @brief Performs an indexed draw call.
 */
struct DrawIndexed {
    /** @brief The start index. */
    u32 first_index;
    /** @brief The number of indices to use. */
    u32 index_count;
};

/**
 * @brief Encapsulates a render related command.
 */
struct RenderCommand {
    union {
        BindGraphicsPipeline bind_graphics_pipeline;
        SetViewport set_viewport;
        BindVertexBuffer bind_vertex_buffer;
        BindIndexBuffer bind_index_buffer;
        BindUniformBuffer bind_uniform_buffer;
        BindUniformBufferRange bind_uniform_buffer_range;
        BindShaderStorageBuffer bind_shader_storage_buffer;
        BindSampledImage bind_sampled_image;
        BindStorageImage bind_storage_image;
        BeginQuery begin_query;
        EndQuery end_query;
        DrawArrays draw_arrays;
        DrawIndexed draw_indexed;
    } command;

    RenderCommandType type;

    /** @brief Attempts to cast the internal command into a Command type. Crashes on fail. */
    template <typename Command>
    auto as() const -> const Command& {
        if constexpr (std::is_same_v<Command, BindGraphicsPipeline>) {
            return command.bind_graphics_pipeline;
        } else if constexpr (std::is_same_v<Command, SetViewport>) {
            return command.set_viewport;
        } else if constexpr (std::is_same_v<Command, BindVertexBuffer>) {
            return command.bind_vertex_buffer;
        } else if constexpr (std::is_same_v<Command, BindIndexBuffer>) {
            return command.bind_index_buffer;
        } else if constexpr (std::is_same_v<Command, BindUniformBuffer>) {
            return command.bind_uniform_buffer;
        } else if constexpr (std::is_same_v<Command, BindUniformBufferRange>) {
            return command.bind_uniform_buffer_range;
        } else if constexpr (std::is_same_v<Command, BindShaderStorageBuffer>) {
            return command.bind_shader_storage_buffer;
        } else if constexpr (std::is_same_v<Command, BindSampledImage>) {
            return command.bind_sampled_image;
        } else if constexpr (std::is_same_v<Command, BindStorageImage>) {
            return command.bind_storage_image;
        } else if constexpr (std::is_same_v<Command, BeginQuery>) {
            return command.begin_query;
        } else if constexpr (std::is_same_v<Command, EndQuery>) {
            return command.end_query;
        } else if constexpr (std::is_same_v<Command, DrawArrays>) {
            return command.draw_arrays;
        } else if constexpr (std::is_same_v<Command, DrawIndexed>) {
            return command.draw_indexed;
        } else {
            static_assert(false, "Invalid Render Command type");
            PANIC("Invalid Render Command. Cannot cast correctly");
        }
    }
};

/// @brief Struct used to initialize and begin a new render pass.
struct RenderPassDescriptor {
    /// @brief An optional label.
    std::optional<std::string> label = std::nullopt;
    /// @brief The @ref RenderTarget to draw to.
    RenderTarget target;
};

struct RenderPass {
    RenderPassDescriptor descriptor;
    std::vector<RenderCommand> commands;
};

class RenderPassRecorder {
public:
    /**
     * @brief Constructs a new @ref RenderPassRecorder.
     * @param descriptor Parameters used to define the render pass.
     * @param size_hint Defines the initial size of the inner command buffer.
     * Use if it is known roughly how many commands will be submitted.
     */
    explicit RenderPassRecorder(const RenderPassDescriptor& descriptor, usize size_hint = 1024);

    /**
     * @brief Binds a @ref GraphicsPipeline to the current render pass.
     * This sets up the state for any following draw commands to this recorder,
     * including shaders used, vertex layout, blend mode etc...
     * @param pipeline_handle The @ref GraphicsPipeline to bind.
     */
    auto bind_graphics_pipeline(const GraphicsPipelineHandle pipeline_handle) noexcept -> void;

    /**
     * @brief Sets the viewport area for all following draw calls.
     * The viewport defines a transformation from NDC to the pixel coords
     * of the render target.
     * If not called, siren defaults to the full render target dimensions.
     * @param x The offset in pixels from the left side of the viewport.
     * @param y The offset in pixels from the top side of the viewport.
     * @param width The width in pixels of the viewport.
     * @param height The height in pixels of the viewport.
     */
    auto set_viewport(const u32 x, const u32 y, const u32 width, const u32 height) noexcept -> void;

    /**
     * @brief Assigns a vertex buffer to a slot.
     * Any following draw calls will use the provided buffer.
     * @note The caller should make sure the @ref Buffer layout matches the
     * layout in the bound @ref GraphicsPipeline.
     * @param buffer The @ref Buffer to bind to the slot.
     * @param slot The slot to bind to.
     * @param offset The offset into the @ref Buffer to start from.
     */
    auto bind_vertex_buffer(const BufferHandle buffer, const u32 slot, const u32 offset) noexcept
        -> void;

    /**
     * @brief Binds an index buffer to the current pass.
     * @note There may only be a single index buffer bound at a time.
     * @param buffer The index buffer to bind.
     * @param index_format The format of the indices (e.g., u8, u16, u32).
     */
    auto bind_index_buffer(const BufferHandle buffer, const IndexFormat index_format) noexcept
        -> void;

    /**
     * @brief Binds a Uniform Buffer to the given slot.
     * @param buffer The @ref Buffer to bind to the slot.
     * @param slot The slot to bind to.
     */
    auto bind_uniform_buffer(const BufferHandle buffer, const u32 slot) noexcept -> void;

    /**
     * @brief Binds a sub range of a Uniform Buffer to the given slot.
     * @param buffer The @ref Buffer to bind to the slot.
     * @param slot The slot to bind to.
     * @param offset The offset into the buffer to start from.
     * @param size The size of the sub range to bind.
     */
    auto bind_uniform_buffer_range(
        const BufferHandle buffer,
        const u32 slot,
        const usize offset,
        const usize size
    ) noexcept -> void;

    /**
     * @brief Binds a Shader Storage Buffer to the given slot.
     * @param buffer The @ref Buffer to bind to the slot.
     * @param slot The slot to bind to.
     */
    auto bind_shader_storage_buffer(const BufferHandle buffer, const u32 slot) noexcept -> void;

    /**
     * @brief Binds an @ref Image to the given slot for sampled access.
     * @param image The @ref Image to bind to the slot.
     * @param sampler The @ref Sampler to access the @ref Image through.
     * @param slot The slot to bind to.
     */
    auto bind_sampled_image(
        const ImageHandle image,
        const SamplerHandle sampler,
        const u32 slot
    ) noexcept -> void;

    /**
     * @brief Binds an @ref Image to the given slot for direct access.
     * @param image The @ref Image to bind to the slot.
     * @param access The access permissions the shader will have.
     * @param slot The slot to bind to.
     */
    auto bind_storage_image(
        const ImageHandle image,
        const AccessKind access,
        const u32 slot
    ) noexcept -> void;

    /**
     * @brief Begins recording query information.
     * @param handle The @ref Query to begin recording information into.
     */
    auto begin_query(const QueryHandle handle) noexcept -> void;

    /**
     * @brief Ends recording query information.
     * @param handle The @ref Query to end recording information into.
     */
    auto end_query(const QueryHandle handle) noexcept -> void;

    /**
     * @brief Draws from the currently bound vertex buffer(s) non indexed.
     * @param start The first vertex to draw.
     * @param count The amount of vertices starting from the first to draw.
     */
    auto draw_arrays(const u32 start, const u32 count) noexcept -> void;

    /**
     * @brief Shorthand function for drawing a fullscreen. Simple uses draw_arrays() under the hood.
     */
    auto draw_fullscreen() noexcept -> void;

    /**
     * @brief Draws from the currently bound vertex buffer(s) using the currently
     * bound index buffer.
     * @param index_count The amount of indices to draw.
     * @param first_index The offset (in indices) into the index buffer to start from.
     */
    auto draw_indexed(const u32 index_count, const u32 first_index) noexcept -> void;

    /// @brief Consumes the RenderPassRecorder. Result should be passed into
    /// @ref RenderCommandRecorder.
    auto finish() -> RenderPass;

private:
    RenderPassDescriptor m_descriptor;
    std::vector<RenderCommand> m_commands;
    GraphicsPipelineHandle m_active_pipeline = NullHandle;
    std::unordered_map<u32, BufferHandle> m_active_vertex_buffers;
    std::unordered_map<u32, BufferHandle> m_active_uniform_buffers;
    std::unordered_map<u32, BufferHandle> m_active_shader_storage_buffers;
    std::unordered_map<u32, ImageHandle> m_sampled_images;
    std::unordered_map<u32, ImageHandle> m_storage_images;
    std::optional<BindIndexBuffer> m_active_index_buffer;
};

} // namespace siren
