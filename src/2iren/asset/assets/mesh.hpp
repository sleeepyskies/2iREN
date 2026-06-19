#pragma once

#include <vector>

#include "../asset.hpp"
#include "../asset_handle.hpp"
#include "pbr_material.hpp"
#include "2iren/base.hpp"
#include "2iren/rhi/resources/buffer.hpp"


namespace siren {

struct Surface : Asset {
    Surface(
        const StrongHandle<PBRMaterialAsset>& material,
        Buffer&& vertex_buffer,
        const Layout& layout,
        Buffer&& index_buffer,
        const IndexFormat index_format,
        const u32 index_count
    ) : material(material),
        index_buffer(std::move(index_buffer)),
        layout(layout),
        index_count(index_count),
        vertex_buffer(std::move(vertex_buffer)),
        index_format(index_format) { }

    /** @brief The name of the surface. */
    std::string name;
    /** @brief The material to use for this surface. */
    StrongHandle<PBRMaterialAsset> material;

    /** @brief The index buffer of this surface. */
    Buffer index_buffer;
    /** @brief The vertex layout of the vertex buffer. */
    Layout layout;
    /** @brief The number of indices this surface has. */
    u32 index_count;

    /** @brief The vertex buffer of this surface. */
    Buffer vertex_buffer;
    /** @brief The format of the indices. */
    IndexFormat index_format;
};

/** @brief A collection of @ref Surface's forming a complete Mesh. */
struct Mesh : Asset {
    /** @brief Name of the mesh. */
    std::string name;
    /** @brief Surfaces belonging to this mesh. */
    std::vector<StrongHandle<Surface>> surfaces;
};

} // namespace siren
