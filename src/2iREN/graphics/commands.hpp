#pragma once

#include <functional>
#include <memory>
#include <unordered_map>

#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/math/color.hpp"
#include "2iREN/math/range.hpp"
#include "2iREN/utility/identifier.hpp"

namespace siren {

/// @brief Indicates the operation to perform on a new pass.
enum class BeginOperation : u8 {
    /// @brief Clears the target a single color.
    Clear,
    /// @brief Keeps the content of the attachment.
    Preserve,
    /// @brief Doesnt even load the previous data.
    Fuckit,
};

/// @brief Indicates the operation to perform for an attachment at the end
/// of a pass.
enum class EndOperation : u8 {
    /// @brief Stores the the rendered contents into the image.
    Store,
    /// @brief The image may be replaced with arbitrary data.
    Fuckit,
};

/// @brief Describes a single color attachment within the context of a
/// render pass.
struct RenderPassColorAttachment {
    /// @brief Handle to the color image.
    ImageHandle    image;
    /// @brief The color to clear the image with if the begin operation is clear.
    Rgba           clear_color;
    /// @brief The action to perform for this attachment at the start of the pass.
    BeginOperation begin_operation;
    /// @brief The action to perform for this attachment at the end of the pass.
    EndOperation   end_operation;
};

/// @brief Simple type alias for a vector of @ref RenderPassColorAttachment.
using RenderPassColorAttachments = std::vector<RenderPassColorAttachment>;

/// @brief Describes the combined depth stencil attachment within the
/// context of a render pass.
struct RenderPassDepthStenctilAttachment {
    /// @brief Handle to the depth stencil image.
    ImageHandle    image;
    /// @brief The depth value to clear the image with if the begin operation
    /// is clear.
    f32            clear_depth;
    /// @brief The stencil value to clear the image with if the begin operation
    /// is clear.
    u32            clear_stencil;
    /// @brief The action to perform for this attachment at the start of the pass.
    BeginOperation begin_operation;
    /// @brief The action to perform for this attachment at the end of the pass.
    EndOperation   end_operation;
};

/// @brief A target of rendering. Essentially a collection of same sized images
/// that can be accessed by shaders during a render pass.
struct RenderTarget {
    /// @brief The color attachments of this target.
    std::vector<RenderPassColorAttachment>           colors;
    /// @brief The combined depth-stencil attachment of this target.
    std::optional<RenderPassDepthStenctilAttachment> depth_stencil;
};

/// @brief Struct used to initialize and begin a new render pass.
/// TODO: can potentially add the following here:
///   - tiling for metal backend
///   - visibility buffers for occlusion culling
struct RenderPassDescriptor {
    /// @brief An optional label.
    Label        label = std::nullopt;
    /// @brief The target used for rendering.
    RenderTarget target;
};

/// @brief Struct used to initialize and begin a new transfer pass.
struct TransferPassDescriptor {
    /// @brief An optional label.
    Label label = std::nullopt;
};

/// TODO: do we need this???
enum class AccessKind {
    ReadOnly,
    WriteOnly,
    ReadWrite,
};

/// @brief Tag to identify the type of a command.
enum class CommandKind : u8 {
    // RENDER COMMANDS

    BindGraphicsPipeline,

    BindVertexBuffer,
    BindIndexBuffer,
    BindUniformBuffer,
    BindUniformBufferRange,
    BindShaderStorageBuffer,

    BindSampledImage,
    BindStorageImage,

    DrawArrays,
    DrawIndexed,

    BeginQuery,
    EndQuery,

    // TRANSFER COMMANDS
    UploadToBuffer,
};

/// @brief Indicates a @ref GraphicsPipeline bind. Sets all of its state.
struct BindGraphicsPipeline {
    /// @brief The pipeline to bind.
    GraphicsPipelineHandle pipeline_handle;
};

/// @brief Binds a vertex buffer to a slot.
struct BindVertexBuffer {
    /// @brief The buffer to bind.
    BufferHandle vertex_buffer;
    /// @brief The slot to bind to
    u32          slot;
    /// @brief The offset into the vertex buffer.
    u32          offset;
};

/// @brief Binds an index buffer.
struct BindIndexBuffer {
    /// @brief The buffer to bind.
    BufferHandle index_buffer;
    /// @brief The format of the indices.
    IndexFormat  index_format;
};

/// @brief Binds a uniform buffer.
struct BindUniformBuffer {
    /// @brief The buffer to bind.
    BufferHandle uniform_buffer;
    /// @brief The slot to bind to.
    u32          slot;
};

/// @brief Binds a subsection of a uniform buffer.
struct BindUniformBufferRange {
    /// @brief The buffer to bind.
    BufferHandle uniform_buffer;
    /// @brief The slot to bind to.
    u32          slot;
    /// @brief The offset in bytes into the buffer to begin the binding range.
    usize        offset;
    /// @brief The size of the sub binding range.
    usize        size;
};

/// @brief Binds a Shader Storage Buffer Object.
struct BindShaderStorageBuffer {
    /// @brief The buffer to bind.
    BufferHandle shader_storage_buffer;
    /// @brief The slot to bind to.
    u32          slot;
};

/// @brief Binds an @ref Image for sampled access. This uses filtering and mipmap
/// sampling. This also allows only for read access and uses texture coordinates
/// instead of pixel coordinates.
struct BindSampledImage {
    /// @brief The @ref Image to bind.
    ImageHandle   image;
    /// @brief The @ref Sampler to use.
    SamplerHandle sampler;
    /// @brief The slot to bind to.
    u32           slot;
};

/// @brief Binds an @ref Image for direct pixel access. This applies no filtering
/// or mip map sampling. This also allows for read write access and uses raw
/// pixel coordinates instead of texture coordinates.
struct BindStorageImage {
    /// @brief The image to bind.
    ImageHandle image;
    /// @brief The slot to bind to.
    u32         slot;
    /// @brief Specifies how the shader may access the @ref Image.
    AccessKind  access;
};

/// @brief Begins recording into a @ref Query.
struct BeginQuery {
    /// @brief The @ref Query to begin.
    QueryHandle query;
};

/// @brief Ends recording into a @ref Query.
struct EndQuery {
    /// @brief The @ref Query to end.
    QueryHandle query;
};

/// @brief Performs a non indexed draw call.
struct DrawArrays {
    /// @brief The start vertex to draw.
    u32 start;
    /// @brief The amount of vertices to draw.
    u32 count;
};

/// @brief Performs an indexed draw call.
struct DrawIndexed {
    /// @brief The start index.
    u32 first_index;
    /// @brief The number of indices to use.
    u32 index_count;
};

/// @brief Uploads data to a buffer.
struct UploadToBuffer {
    /// @brief The buffer to upload data to.
    BufferHandle   buffer;
    /// @brief A non owning view of the data to upload.
    /// @warning The caller must ensure this data remains alive!
    ByteBufferView data;
    /// @brief The offset into the destination buffer to start uploading at.
    u32            offset;
};

/// @brief Encapsulates a GPU command.
struct Command {
    union {
        // RENDER COMMANDS
        BindGraphicsPipeline    bind_graphics_pipeline;
        BindVertexBuffer        bind_vertex_buffer;
        BindIndexBuffer         bind_index_buffer;
        BindUniformBuffer       bind_uniform_buffer;
        BindUniformBufferRange  bind_uniform_buffer_range;
        BindShaderStorageBuffer bind_shader_storage_buffer;
        BindSampledImage        bind_sampled_image;
        BindStorageImage        bind_storage_image;
        BeginQuery              begin_query;
        EndQuery                end_query;
        DrawArrays              draw_arrays;
        DrawIndexed             draw_indexed;

        // TRANSFER COMMANDS
        UploadToBuffer upload_to_buffer;
    } command;

    CommandKind type;

    /// @brief Attempts to cast the internal command into a Command type.
    /// Crashes on fail.
    template <typename Command>
    constexpr auto as() const -> const Command& {
        // REDNER COMMANDS
        if constexpr (std::is_same_v<Command, BindGraphicsPipeline>) {
            return command.bind_graphics_pipeline;
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
            // TRANSFER COMMANDS
        } else if constexpr (std::is_same_v<Command, UploadToBuffer>) {
            return command.upload_to_buffer;
        } else {
            static_assert(false, "Invalid Render Command type");
            PANIC("Invalid Render Command. Cannot cast correctly");
        }
    }
};

/// @brief Simple type alias for a vector of commands.
using Commands = std::vector<Command>;

/// @brief Used to record a series of render commands form a render pass.
/// This records a purely CPU side series of commands. GPU execution is
/// deferred until later.
/// May also perform some light, non-destructive optimizations on the commands
/// recorded.
class RenderCommandRecorder {
    friend class CommandRecorder;

    explicit RenderCommandRecorder(const Device* device);

public:
    /// @brief Binds a @ref GraphicsPipeline to the current render pass.
    /// This sets up the state for any following draw commands to this recorder,
    /// including shaders used, vertex layout, blend mode etc...
    /// @param pipeline The @ref GraphicsPipeline to bind.
    auto bind_graphics_pipeline(const GraphicsPipelineHandle pipeline) noexcept -> void;

    /// @brief Assigns a vertex buffer to a slot.
    /// Any following draw calls will use the provided buffer.
    /// @note The caller should make sure the @ref Buffer layout matches the
    /// layout in the bound @ref GraphicsPipeline.
    /// @param buffer The @ref Buffer to bind to the slot.
    /// @param slot The slot to bind to.
    /// @param offset The offset into the @ref Buffer to start from.
    auto bind_vertex_buffer(const BufferHandle buffer, const u32 slot, const u32 offset) noexcept
        -> void;

    /// @brief Binds an index buffer to the current pass.
    /// @note There may only be a single index buffer bound at a time.
    /// @param buffer The index buffer to bind.
    /// @param index_format The format of the indices (e.g., u8, u16, u32).
    auto bind_index_buffer(const BufferHandle buffer, const IndexFormat index_format) noexcept
        -> void;

    /// @brief Binds a Uniform Buffer to the given slot.
    /// @param buffer The @ref Buffer to bind to the slot.
    /// @param slot The slot to bind to.
    auto bind_uniform_buffer(const BufferHandle buffer, const u32 slot) noexcept -> void;

    /// @brief Binds a sub range of a Uniform Buffer to the given slot.
    /// @param buffer The @ref Buffer to bind to the slot.
    /// @param slot The slot to bind to.
    /// @param offset The offset into the buffer to start from.
    /// @param size The size of the sub range to bind.
    auto bind_uniform_buffer_range(
        const BufferHandle buffer,
        const u32          slot,
        const usize        offset,
        const usize        size
    ) noexcept -> void;

    /// @brief Binds a Shader Storage Buffer to the given slot.
    /// @param buffer The @ref Buffer to bind to the slot.
    /// @param slot The slot to bind to.
    auto bind_shader_storage_buffer(const BufferHandle buffer, const u32 slot) noexcept -> void;

    /// @brief Binds an @ref Image to the given slot for sampled access.
    /// @param image The @ref Image to bind to the slot.
    /// @param sampler The @ref Sampler to access the @ref Image through.
    /// @param slot The slot to bind to.
    auto bind_sampled_image(
        const ImageHandle   image,
        const SamplerHandle sampler,
        const u32           slot
    ) noexcept -> void;

    /// @brief Binds an @ref Image to the given slot for direct access.
    /// @param image The @ref Image to bind to the slot.
    /// @param access The access permissions the shader will have.
    /// @param slot The slot to bind to.
    auto bind_storage_image(
        const ImageHandle image,
        const AccessKind  access,
        const u32         slot
    ) noexcept -> void;

    /// @brief Begins recording query information.
    /// @param handle The @ref Query to begin recording information into.
    auto begin_query(const QueryHandle handle) noexcept -> void;

    /// @brief Ends recording query information.
    /// @param handle The @ref Query to end recording information into.
    auto end_query(const QueryHandle handle) noexcept -> void;

    /// @brief Draws from the currently bound vertex buffer(s) non indexed.
    /// @param start The first vertex to draw.
    /// @param count The amount of vertices starting from the first to draw.
    auto draw_arrays(const u32 start, const u32 count) noexcept -> void;

    /// @brief Draws from the currently bound vertex buffer(s) using the
    /// currently bound index buffer.
    /// @param index_count The amount of indices to draw.
    /// @param first_index The offset (in indices) into the index buffer to start
    /// from.
    auto draw_indexed(const u32 index_count, const u32 first_index) noexcept -> void;

private:
    /// @brief Consumes this RenderCommandRecorder and returns the collected commands.
    auto finish() && -> Commands;

    std::vector<Command> m_commands;

    GraphicsPipelineHandle                m_active_pipeline = NullHandle;
    std::unordered_map<u32, BufferHandle> m_active_vertex_buffers;
    std::unordered_map<u32, BufferHandle> m_active_uniform_buffers;
    std::unordered_map<u32, BufferHandle> m_active_shader_storage_buffers;
    std::unordered_map<u32, ImageHandle>  m_sampled_images;
    std::unordered_map<u32, ImageHandle>  m_storage_images;
    std::optional<BindIndexBuffer>        m_active_index_buffer;
};

/// @brief Handles recording any data trnasferal commands into a command list.
/// @warning Most commands make use of non owning views into CPU buffers.
/// Therefore the called should make sure to keep the CPU data alive until the
/// corresponding CommandList in which this will record commands into has been
/// submitted.
class TransferCommandRecorder {
    friend class CommandRecorder;

    explicit TransferCommandRecorder(const Device* device);

public:
    /// @brief Uploads data from the provided buffer view into a GPU buffer.
    /// @param data A non owning view of the CPU data to upload to the GPU.
    /// @param offset The offset in bytes into the GPU buffer from which the data
    /// will be uploaded.
    /// @warning The @param `data` must be kept alive until the command list
    /// has been submit!
    auto upload_to_buffer(BufferHandle buffer, ByteBufferView data, u32 offset) -> void;

private:
    /// @brief Consumes this TransferCommandRecorder and returns the collected
    /// commands.
    [[nodiscard]]
    auto finish() && -> Commands;

    std::vector<Command> m_commands;
};

/// @brief The result of calling finish on a CommandRecorder.
struct CommandList {
    struct Pass {
        ~Pass() {
            kind == Kind::Render ? std::destroy_at(&descriptor.render_descriptor)
                                 : std::destroy_at(&descriptor.transfer_descriptor);
        }

        Pass(RenderPassDescriptor descriptor, Range<usize> range) :
            command_range(range), kind(Kind::Render) {
            std::construct_at(&this->descriptor.render_descriptor, std::move(descriptor));
        }

        Pass(TransferPassDescriptor descriptor, Range<usize> range) :
            command_range(range), kind(Kind::Transfer) {
            std::construct_at(&this->descriptor.transfer_descriptor, std::move(descriptor));
        }

        Pass(Pass&& other) noexcept :
            command_range(std::move(other.command_range)), kind(other.kind) {
            if (kind == Kind::Render) {
                std::construct_at(
                    &descriptor.render_descriptor, std::move(other.descriptor.render_descriptor)
                );
            } else {
                std::construct_at(
                    &descriptor.transfer_descriptor, std::move(other.descriptor.transfer_descriptor)
                );
            }
        }

        Pass(const Pass&)            = delete;
        Pass& operator=(const Pass&) = delete;

        union Descriptor {
            RenderPassDescriptor   render_descriptor;
            TransferPassDescriptor transfer_descriptor;
            Descriptor() { }
            ~Descriptor() { }
        } descriptor;
        Range<usize> command_range;
        enum class Kind : u8 { Render, Transfer } kind;
    };

    /// @brief Returns a view over the commands within the provided range.
    [[nodiscard]]
    auto command_view(const Range<usize>& range) -> std::span<const Command> {
        // [0, 1, 2, 3, 4, 5]
        // we do view(2, 4)
        return std::span(commands).subspan(range.begin, range.end - range.begin);
    }

    /// @brief List of passes describing how to group the commands.
    std::vector<Pass>    passes;
    /// @brief The list of raw commands.
    std::vector<Command> commands;
};

/// @brief A function passed into the @ref RenderCommandRecorder that
/// will record commands to.
using RenderPassFunction   = std::function<void(RenderCommandRecorder&)>;
/// @brief A function passed into the @ref TransferCommandRecorder that
/// will record commands to.
using TransferPassFunction = std::function<void(TransferCommandRecorder&)>;

class CommandRecorder {
    friend class Device;

public:
    auto render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& func) -> void;

    auto transfer_pass(const TransferPassDescriptor& descriptor, TransferPassFunction&& func)
        -> void;

    [[nodiscard]]
    auto finish() && -> CommandList;

private:
    CommandRecorder(const Device* device);

    const Device*                  m_device;
    std::vector<CommandList::Pass> m_passes   = {};
    std::vector<Command>           m_commands = {};
};

} // namespace siren
