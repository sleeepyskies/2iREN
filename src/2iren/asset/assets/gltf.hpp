#pragma once

#include <vector>

#include "2iren/asset/fwd.hpp"
#include "mesh.hpp"
#include "pbr_material.hpp"
#include "2iren/base.hpp"
#include "2iren/asset/asset_handle.hpp"
#include "2iren/asset/asset_loader.hpp"


namespace siren {

/** @brief A single node in the scene hierarchy. */
struct GltfNode : Asset {
    /** @brief The name of the node. */
    std::string name;
    /** @brief The index of the node in the main gltf asset. */
    usize index;
    /** @brief The transform of the node. */
    glm::mat4 transform;
    /** @brief The mesh of the node. */
    std::optional<StrongHandle<Mesh>> mesh;
    /** @brief The parent node of the node (if present). */
    std::optional<WeakHandle> parent;
    /** @brief The children nodes of the node. */
    std::vector<StrongHandle<GltfNode>> children;
};

/** @brief A collection of nodes. Similar to a prefab. */
struct GltfScene : Asset {
    GltfScene(
        const std::string& name,
        const usize index,
        std::vector<StrongHandle<GltfNode>>&& root_nodes
    ) : name(name),
        index(index),
        root_nodes(std::move(root_nodes)) { }

    /** @brief The name of the scene. */
    std::string name;
    /** @brief The index of the scene in the main gltf asset. */
    usize index;
    /** @brief The root nodes of the scene. */
    std::vector<StrongHandle<GltfNode>> root_nodes;
};

/** @brief Represents a loaded gltf/glb file. */
struct Gltf : Asset {
    Gltf(
        std::vector<StrongHandle<GltfScene>>&& scenes,
        std::optional<StrongHandle<GltfScene>>&& default_scene,
        std::vector<StrongHandle<Mesh>>&& meshes,
        std::vector<StrongHandle<PBRMaterialAsset>>&& materials,
        std::vector<StrongHandle<GltfNode>>&& nodes
    ) : scenes(std::move(scenes)),
        default_scene(std::move(default_scene)),
        meshes(std::move(meshes)),
        materials(std::move(materials)),
        nodes(std::move(nodes)) { }

    /** @brief All scenes loaded from the gltf. */
    std::vector<StrongHandle<GltfScene>> scenes;
    /** @brief The default scene, if provided. */
    std::optional<StrongHandle<GltfScene>> default_scene;
    /** @brief All meshes loaded from the gltf. */
    std::vector<StrongHandle<Mesh>> meshes;
    /** @brief All materials loaded from the gltf. */
    std::vector<StrongHandle<PBRMaterialAsset>> materials;
    /** @brief All textures loaded from the gltf. */
    std::vector<StrongHandle<Texture>> textures;
    /** @brief All nodes loaded from the gltf. */
    std::vector<StrongHandle<GltfNode>> nodes;
};

template <>
struct LoaderTraits<Gltf> {
    /** @brief Loader configuration for loading gltf files. */
    struct Config {
        // bool triangulate_meshes = true;
    };
};

/**
 * @brief Loader for gltf files. Loads gltf assets and all sub-assets.
 * See @ref Gltf
 */
class GltfLoader final : public AssetLoader<Gltf> {
public:
    auto load(
        LoadContext&& ctx,
        std::optional<ConfigType> config
    ) const -> AssetLoadError override;
    auto extensions() const -> std::vector<std::string_view> override { return { "glb", "gltf" }; }
};

} // namespace siren
