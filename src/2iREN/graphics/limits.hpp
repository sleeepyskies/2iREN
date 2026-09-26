#pragma once

#include "2iREN/core/base.hpp"

namespace siren {

/// @brief Defines the hardware limits of the current backend.
struct Limits {
    /// @brief Maximum size, in bytes, of a single shader storage block.
    u32 max_shader_storage_block_size;
    /// @brief Required byte alignment for uniform buffer binding offsets.
    u32 uniform_buffer_offset_alignment;
    /// @brief Required byte alignment for shader storage buffer binding offsets.
    u32 shader_storage_buffer_offset_alignment;
};

} // namespace siren
