#pragma once

#include "2iREN/core/base.hpp"
#include "2iREN/math/vec3.hpp"

namespace siren {
/// @brief Defines the hardware limits of the current backend.
struct Limits {
    /// @brief Maximum number of uniform buffer binding points.
    u32 max_uniform_buffer_bindings;
    /// @brief Maximum number of shader storage buffer binding points.
    u32 max_shader_storage_buffer_bindings;
    /// @brief Maximum size, in bytes, of a single uniform block.
    u32 max_uniform_block_size;
    /// @brief Maximum size, in bytes, of a single shader storage block.
    u32 max_shader_storage_block_size;
    /// @brief Required byte alignment for uniform buffer binding offsets.
    u32 uniform_buffer_offset_alignment;
    /// @brief Required byte alignment for shader storage buffer binding offsets.
    u32 shader_storage_buffer_offset_alignment;
    /// @brief Maximum number of vertex attributes supported by the pipeline.
    u32 max_vertex_attributes;
    /// @brief Maximum width or height of a 2D texture, in texels.
    u32 max_texture_size;
    /// @brief Maximum number of layers in a texture array.
    u32 max_array_texture_layers;
    /// @brief Maximum number of texture units accessible across all shader stages.
    u32 max_texture_units;
    /// @brief Maximum number of color attachments supported by a framebuffer.
    u32 max_color_attachments;
    /// @brief Maximum number of color attachments that can be written to in a single draw call.
    u32 max_draw_buffers;
    /// @brief Maximum supported multisample sample count.
    u32 max_samples;
    /// @brief Maximum number of local invocations in a compute work group.
    u32 max_compute_work_group_invocations;
    /// @brief Maximum work group counts along the X, Y, and Z dimensions.
    Vec3u max_compute_work_group_count;
    /// @brief Maximum local work group sizes along the X, Y, and Z dimensions.
    Vec3u max_compute_work_group_size;
};

} // namespace siren
