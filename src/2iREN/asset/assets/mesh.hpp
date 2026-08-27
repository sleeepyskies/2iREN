#pragma once

#include <vector>

#include "../asset_handle.hpp"
#include "2iREN/base.hpp"
#include "2iREN/rhi/resources/buffer.hpp"
#include "material.hpp"

// todo: do we want to store buffer data CPU side as well?


namespace siren {

/**
 * @struct IndexBuffer
 * @brief Simple container semantically representing an index buffer.
 */
struct IndexBuffer {
    /** @brief The GPU side index data. */
    Buffer buffer;
    /** @brief The number of indices. */
    usize count;
    /** @brief The type of the indices. */
    IndexFormat format;
};

/**
 * @struct VertexBuffer
 * @brief Simple container semantically representing a vertex buffer.
 */
struct VertexBuffer {
    /** @brief The GPU buffer of the vertex data. */
    Buffer buffer;
    /** @brief Describes the layout of the vertex buffer. */
    Layout layout;
};

struct Surface : Asset {
    Surface(
        const std::string& name,
        const StrongHandle<MaterialAsset>& material,
        IndexBuffer&& index_buffer,
        VertexBuffer&& vertex_buffer
    ) : name(name),
        material(material),
        index_buffer(std::move(index_buffer)),
        vertex_buffer(std::move(vertex_buffer)) { }

    /** @brief The name of the surface. */
    std::string name;
    /** @brief The material to use for this surface. */
    StrongHandle<MaterialAsset> material;
    /** @brief The indices of the Surface. */
    IndexBuffer index_buffer;
    /** @brief The vertices of the Surface. */
    VertexBuffer vertex_buffer;
};

/** @brief A collection of @ref Surface's forming a complete Mesh. */
struct Mesh : Asset {
    /** @brief Name of the mesh. */
    std::string name;
    /** @brief Surfaces belonging to this mesh. */
    std::vector<StrongHandle<Surface>> surfaces;
};

} // namespace siren
