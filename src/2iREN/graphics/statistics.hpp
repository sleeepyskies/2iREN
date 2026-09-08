#pragma once

#include "2iREN/core/base.hpp"

namespace siren {

/// @brief Rendering statistics detailing the amount of as well as the types
/// of operations performed by backend.
struct Statistics {
    u32 count_bind_graphics_pipeline;
    u32 count_set_viewport;
    u32 count_bind_vertex_buffer;
    u32 count_bind_index_buffer;
    u32 count_bind_uniform_buffer;
    u32 count_bind_shader_storage_buffer;
    u32 count_bind_sampled_image;
    u32 count_bind_storage_image;
    u32 count_draw_arrays;
    u32 count_draw_indexed;
    u32 count_upload_buffer;
    u32 count_upload_image;
    u32 count_draw_calls;
    u32 count_render_passes;

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
        count_draw_calls += rhs.count_draw_calls;
        count_render_passes += rhs.count_render_passes;
        return *this;
    }
};

[[nodiscard]]
constexpr auto operator+(Statistics lhs, const Statistics& rhs) noexcept -> Statistics {
    lhs += rhs;
    return lhs;
}

} // namespace siren
