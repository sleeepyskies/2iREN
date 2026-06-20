#include "gltf.hpp"

#include <expected>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "2iren/asset/asset_server.hpp"
#include "2iren/rhi/device.hpp"
#include "2iren/util/cgltf.cpp"
#include "2iren/util/filesystem.hpp"
#include "2iren/util/stb_image.cpp"

/// For docs on GLTF see: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#indices-and-names
/// For a brief overview of GLTF see: https://www.khronos.org/files/gltf20-reference-guide.pdf

// todo: LOGGING PLESE WOULD HELP SO MUCH HERE

struct NameIDGenerator {
    std::string fallback = "Unnamed";
    siren::u32 count   = 0;
    auto next(const char* name) -> std::string {
        if (name) { return name; }
        return fallback + "_" + std::to_string(count++);
    }
};

namespace siren {
// ============================================================================
// == MARK: Mappings
// ============================================================================
static auto gltf_attribute_to_siren(const u32 attribute) -> Attribute {
    switch (attribute) {
        case cgltf_attribute_type_position: return Attribute::Position;
        case cgltf_attribute_type_normal: return Attribute::Normal;
        case cgltf_attribute_type_tangent: return Attribute::Tangent;
        case cgltf_attribute_type_texcoord: return Attribute::Texture;
        case cgltf_attribute_type_color: return Attribute::Color;
        case cgltf_attribute_type_joints:
        case cgltf_attribute_type_weights:
        case cgltf_attribute_type_custom: PANIC("Unsupported cgltf attribute type encountered.");
        case cgltf_attribute_type_invalid:
        case cgltf_attribute_type_max_enum:
        default: UNREACHABLE("Could not convert cgltf attribute type to native 2iren type.");
    }
}

static auto gltf_filter_to_siren(const i32 filter) -> ImageFilterMode {
    switch (filter) {
        // opengl/gltf combine min filter and mipmap filter
        case cgltf_filter_type_nearest_mipmap_linear:
        case cgltf_filter_type_nearest_mipmap_nearest:
        case cgltf_filter_type_nearest: return ImageFilterMode::Nearest;

        case cgltf_filter_type_linear_mipmap_nearest:
        case cgltf_filter_type_linear_mipmap_linear:
        case cgltf_filter_type_linear: return ImageFilterMode::Linear;

        case cgltf_filter_type_undefined: PANIC("These filter types are not supported yet.");
        default: UNREACHABLE("Could not convert cgltf filter type to native 2iren type.");
    }
}

static auto gltf_mipmap_filter_to_siren(const i32 filter) -> ImageFilterMode {
    switch (filter) {
        // opengl/gltf combine min filter and mipmap filter

        case cgltf_filter_type_nearest_mipmap_nearest:
        case cgltf_filter_type_linear_mipmap_nearest: return ImageFilterMode::Nearest;

        case cgltf_filter_type_nearest_mipmap_linear:
        case cgltf_filter_type_linear_mipmap_linear: return ImageFilterMode::Linear;

        // doesnt specify any filtering for mipmaps
        case cgltf_filter_type_linear:
        case cgltf_filter_type_nearest: return ImageFilterMode::None;

        case cgltf_filter_type_undefined: PANIC("These filter types are not supported yet.");
        default: UNREACHABLE("Could not convert cgltf mipmap filter type to native 2iren type.");
    }
}

static auto gltf_wrap_to_siren(const i32 wrap) -> ImageWrapMode {
    switch (wrap) {
        case cgltf_wrap_mode_clamp_to_edge: return ImageWrapMode::ClampEdge;
        case cgltf_wrap_mode_mirrored_repeat: return ImageWrapMode::Mirror;
        case cgltf_wrap_mode_repeat: return ImageWrapMode::Repeat;
        default: UNREACHABLE("Could not convert cgltf wrap mode to native 2iren type.");
    }
}

static auto gltf_alpha_mode_to_siren(const i32 alpha_mode) -> AlphaMode {
    switch (alpha_mode) {
        case cgltf_alpha_mode_opaque: return AlphaMode::Opaque;
        case cgltf_alpha_mode_mask: return AlphaMode::Mask;
        case cgltf_alpha_mode_blend: return AlphaMode::Blend;
        default: UNREACHABLE("Could not convert cgltf alpha mode to native 2iren type.");
    }
}

// ============================================================================
// == MARK: Helper functions
// ============================================================================

static auto read_buffer_data(
    const u8* buf,
    const u32 offset,
    const u32 count,
    const u32 stride,
    const u32 component_size,
    const u32 num_components
) -> std::vector<u8> {
    std::vector<u8> vec;
    const u32 elem_size = component_size * num_components;
    const u32 stride_   = stride == 0 ? elem_size : stride; // if 0, data is tightly packed
    vec.resize(count * elem_size);

    u32 dest_offset = 0;
    u32 src_offset  = offset;
    for (u32 i = 0; i < count; i++) {
        std::memcpy(vec.data() + dest_offset, buf + src_offset, elem_size);
        dest_offset += elem_size;
        src_offset += stride_;
    }

    return vec;
}

static auto parse_sampler(const cgltf_sampler* sampler, Device& device) -> Sampler {
    // use default sampler if none is provided.
    SamplerDescriptor desc;
    if (sampler) {
        desc.min_filter = gltf_filter_to_siren(sampler->min_filter);
        desc.max_filter = gltf_filter_to_siren(sampler->mag_filter);
        desc.mipmap_filter = gltf_mipmap_filter_to_siren(sampler->min_filter);
        desc.s_wrap = gltf_wrap_to_siren(sampler->wrap_s);
        desc.t_wrap = gltf_wrap_to_siren(sampler->wrap_t);
        // the following are not provided by gltf spec:
        // r_wrap, lod_min, lod_max, border_color, compare_mode, compare_fn
    }
    return device.create_sampler(std::move(desc));
}

static auto load_textures(
    const cgltf_data* data,
    LoadContext& ctx
) -> std::expected<std::vector<StrongHandle<Texture>>, AssetErrorCode> {
    NameIDGenerator name_gen{ .fallback = "Texture" };

    std::vector<StrongHandle<Texture>> vec;
    vec.reserve(data->textures_count);

    for (usize texture_idx = 0; texture_idx < data->textures_count; texture_idx++) {
        const auto& texture = data->textures[texture_idx];
        auto sampler        = parse_sampler(texture.sampler, ctx.device());
        const auto name     = name_gen.next(texture.name);

        // gltf textures can be either embedded into the gltf
        // via a buffer, or stored elsewhere on disk and be
        // provided via the uri. bufferView and uri for texture
        // are mutually exclusive
        // I think the data could also be embedded directly in
        // the uri, but siren doesn't account for this.

        auto handle = StrongHandle<Texture>::invalid();
        if (texture.image->uri) {
            // load image from disk => spawn async task using ImageLoader
            const auto path = FileSystem::to_virtual(texture.image->uri, ctx.path().vfs());
            if (!path) { return std::unexpected(AssetErrorCode::FileNotFound); }
            handle = ctx.load_external_asset<Texture>(
                path.value().string(),
                std::make_optional(
                    TextureLoader::ConfigType{
                        .name = name,
                        .format = ImageFormat{},
                        .sampler = std::move(sampler),
                        .array_layout = ImageArrayLayout{},
                        .is_srgb = false,
                        .generate_mipmap_levels = true,
                    }
                )
            );
        } else if (texture.image->buffer_view) {
            // load image from buffer
            if (texture.image->buffer_view->has_meshopt_compression) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }
            const u8* bytes = cgltf_buffer_view_data(texture.image->buffer_view);
            const auto size = texture.image->buffer_view->size;

            i32 width, height, channels;
            std::unique_ptr<u8, void(*)(void*)> img_data(
                stbi_load_from_memory(bytes, (int)size, &width, &height, &channels, STBI_default),
                stbi_image_free
            );
            if (!img_data) { return std::unexpected(AssetErrorCode::AssetCorrupted); }

            const auto format = channels == 1
                                    ? ImageFormat::Mask8
                                    : channels == 4
                                          ? ImageFormat::LinearColor8
                                          : ImageFormat::Unknown;

            const auto extent = ImageExtent{
                .width = (u32)width,
                .height = (u32)height,
                .depth_or_layers = 1
            };

            const u32 max_dim       = std::max({ extent.width, extent.height, extent.depth_or_layers });
            const u32 mipmap_levels = 1 + static_cast<u32>(glm::floor(glm::log2(max_dim)));

            // todo: add name?
            auto img = ctx.device().create_image({
                .label = std::nullopt,
                .format = format,
                .extent = extent,
                .dimension = ImageDimension::D2,
                .mipmap_levels = mipmap_levels,
            });
            handle = ctx.add_labeled_asset<Texture>(
                name,
                std::make_unique<Texture>(
                    name,
                    std::move(img),
                    std::move(sampler)
            ));
        } else {
            return std::unexpected(AssetErrorCode::AssetCorrupted);
        }

        if (!handle.is_valid()) { return std::unexpected(AssetErrorCode::InvalidFormat); }
        vec.emplace_back(std::move(handle));
    }

    return vec;
}

static auto load_materials(
    const cgltf_data* data,
    const std::vector<StrongHandle<Texture>>& textures,
    LoadContext& ctx
) -> std::expected<std::vector<StrongHandle<PBRMaterialAsset>>, AssetErrorCode> {
    NameIDGenerator name_gen{ .fallback = "Material" };

    const auto get_texture = [&textures, &data] (
        const cgltf_texture* texture
    ) -> std::expected<StrongHandle<Texture>, AssetErrorCode>{
        const usize idx = texture - data->textures;
        if (idx > textures.size()) {
            return std::unexpected(AssetErrorCode::AssetCorrupted);
        }
        return textures[idx];
    };

    std::vector<StrongHandle<PBRMaterialAsset>> vec;
    vec.reserve(data->materials_count);

    for (usize material_idx = 0; material_idx < data->materials_count; material_idx++) {
        const auto& gltf_material = data->materials[material_idx];
        const auto name           = name_gen.next(gltf_material.name);
        auto mat                  = std::make_unique<PBRMaterialAsset>(name);

        // metallic roughness and specular glossiness are mutually exclusive. we stick to just metallic roughness

        if (gltf_material.has_pbr_metallic_roughness) {
            const auto& pbr_mr = gltf_material.pbr_metallic_roughness;
            mat->set_base_color(RGBA{glm::make_vec4(pbr_mr.base_color_factor)});
            mat->set_metallic(pbr_mr.metallic_factor);
            mat->set_roughness(pbr_mr.roughness_factor);
            if (pbr_mr.base_color_texture.texture) {
                const auto texture = get_texture(pbr_mr.base_color_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_base_color_tex(texture.value());
            }
            if (pbr_mr.metallic_roughness_texture.texture) {
                const auto texture = get_texture(pbr_mr.metallic_roughness_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_metallic_roughness_tex(texture.value());
            }
        }

        if (gltf_material.has_pbr_specular_glossiness) { /* pass */ }

        if (gltf_material.has_clearcoat) {
            const auto& cc = gltf_material.clearcoat;
            mat->set_clear_coat(cc.clearcoat_factor);
            mat->set_clear_coat_roughness(cc.clearcoat_roughness_factor);
            if (cc.clearcoat_texture.texture) {
                const auto texture = get_texture(cc.clearcoat_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_clear_coat_tex(texture.value());
            }
            if (cc.clearcoat_roughness_texture.texture) {
                const auto texture = get_texture(cc.clearcoat_roughness_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_clear_coat_tex(texture.value());
            }
            if (cc.clearcoat_normal_texture.texture) {
                const auto texture = get_texture(cc.clearcoat_normal_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_clear_coat_tex(texture.value());
            }
        }

        if (gltf_material.has_transmission) {
            const auto& tr = gltf_material.transmission;
            mat->set_transmission(tr.transmission_factor);
            if (tr.transmission_texture.texture) {
                const auto texture = get_texture(tr.transmission_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_transmission_tex(texture.value());
            }
        }

        if (gltf_material.has_volume) {
            const auto& vol = gltf_material.volume;
            mat->set_thickness(vol.thickness_factor);
            mat->set_attenuation_color(glm::make_vec3(vol.attenuation_color));
            mat->set_attenuation_distance(vol.attenuation_distance);
            if (vol.thickness_texture.texture) {
                const auto texture = get_texture(vol.thickness_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_thickness_texture(texture.value());
            }
        }

        if (gltf_material.has_ior) {
            mat->set_ior(gltf_material.ior.ior);
        }

        if (gltf_material.has_specular) {
            const auto& spec = gltf_material.specular;
            mat->set_specular_factor(spec.specular_factor);
            mat->set_specular_color(glm::make_vec3(spec.specular_color_factor));
            if (spec.specular_color_texture.texture) {
                const auto texture = get_texture(spec.specular_color_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_specular_color_tex(texture.value());
            }
            if (spec.specular_texture.texture) {
                const auto texture = get_texture(spec.specular_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_specular_tex(texture.value());
            }
        }

        if (gltf_material.has_sheen) {
            const auto& sh = gltf_material.sheen;
            mat->set_sheen_color(glm::make_vec3(sh.sheen_color_factor));
            mat->set_sheen_roughness(sh.sheen_roughness_factor);
            if (sh.sheen_color_texture.texture) {
                const auto texture = get_texture(sh.sheen_color_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_sheen_color_tex(texture.value());
            }
            if (sh.sheen_roughness_texture.texture) {
                const auto texture = get_texture(sh.sheen_roughness_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_sheen_roughness_tex(texture.value());
            }
        }

        if (gltf_material.has_emissive_strength) {
            mat->set_emissive_strength(gltf_material.emissive_strength.emissive_strength);
        }

        if (gltf_material.has_iridescence) {
            const auto& ir = gltf_material.iridescence;
            mat->set_iridescence_factor(ir.iridescence_factor);
            mat->set_iridescence_ior(ir.iridescence_ior);
            mat->set_iridescence_min(ir.iridescence_thickness_min);
            mat->set_iridescence_max(ir.iridescence_thickness_max);
            if (ir.iridescence_texture.texture) {
                const auto texture = get_texture(ir.iridescence_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_iridescence_tex(texture.value());
            }
            if (ir.iridescence_thickness_texture.texture) {
                const auto texture = get_texture(ir.iridescence_thickness_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_iridescence_thickness_tex(texture.value());
            }
        }

        if (gltf_material.has_diffuse_transmission) {
            const auto& df = gltf_material.diffuse_transmission;
            mat->set_diffuse_transmission_factor(df.diffuse_transmission_factor);
            mat->set_diffuse_transmission_color(glm::make_vec3(df.diffuse_transmission_color_factor));
            if (df.diffuse_transmission_texture.texture) {
                const auto texture = get_texture(df.diffuse_transmission_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_diffuse_transmission_tex(texture.value());
            }
            if (df.diffuse_transmission_color_texture.texture) {
                const auto texture = get_texture(df.diffuse_transmission_color_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_diffuse_transmission_color_tex(texture.value());
            }
        }

        if (gltf_material.has_anisotropy) {
            const auto& an = gltf_material.anisotropy;
            mat->set_anisotropy_strength(an.anisotropy_strength);
            mat->set_anisotropy_rotation(an.anisotropy_rotation);
            if (an.anisotropy_texture.texture) {
                const auto texture = get_texture(an.anisotropy_texture.texture);
                if (!texture.has_value()) { return std::unexpected(texture.error()); }
                mat->set_anisotropy_tex(texture.value());
            }
        }

        if (gltf_material.has_dispersion) {
            mat->set_dispersion(gltf_material.dispersion.dispersion);
        }

        if (gltf_material.normal_texture.texture) {
            const auto texture = get_texture(gltf_material.normal_texture.texture);
            if (!texture.has_value()) { return std::unexpected(texture.error()); }
            mat->set_normal_tex(texture.value());
        }

        if (gltf_material.occlusion_texture.texture) {
            const auto texture = get_texture(gltf_material.occlusion_texture.texture);
            if (!texture.has_value()) { return std::unexpected(texture.error()); }
            mat->set_occlusion_tex(texture.value());
        }

        if (gltf_material.emissive_texture.texture) {
            const auto texture = get_texture(gltf_material.emissive_texture.texture);
            if (!texture.has_value()) { return std::unexpected(texture.error()); }
            mat->set_emissive_tex(texture.value());
        }

        mat->set_emissive_color(glm::make_vec3(gltf_material.emissive_factor));

        mat->set_alpha_mode(gltf_alpha_mode_to_siren(gltf_material.alpha_mode));
        mat->set_alpha_cutoff(gltf_material.alpha_cutoff);
        mat->set_double_sided(gltf_material.double_sided);
        mat->set_unlit(gltf_material.unlit);

        vec.emplace_back(ctx.add_labeled_asset(name, std::move(mat)));
    }

    return vec;
}

static auto load_meshes(
    const cgltf_data* data,
    const std::vector<StrongHandle<PBRMaterialAsset>>& materials,
    LoadContext& ctx
) -> std::expected<std::vector<StrongHandle<Mesh>>, AssetErrorCode> {
    // todo: i think the whole buffer reading logic needs to be redone lmao. for like the 4th time now

    NameIDGenerator name_gen{ .fallback = "Mesh" };

    std::vector<StrongHandle<Mesh>> vec;
    vec.reserve(data->meshes_count);

    for (usize mesh_idx = 0; mesh_idx < data->meshes_count; mesh_idx++) {
        const auto& gltf_mesh = data->meshes[mesh_idx];
        const auto name       = name_gen.next(gltf_mesh.name);
        std::vector<StrongHandle<Surface>> surfaces;
        surfaces.reserve(gltf_mesh.primitives_count);
        auto mesh  = std::make_unique<Mesh>();
        mesh->name = name;

        for (i32 prim_idx = 0; prim_idx < gltf_mesh.primitives_count; prim_idx++) {
            const auto& gltf_prim = gltf_mesh.primitives[prim_idx];
            // only support triangles
            if (gltf_prim.type != cgltf_primitive_type_triangles) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }

            // dont support any compression
            if (gltf_prim.has_draco_mesh_compression) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }

            const auto gltf_indices = gltf_prim.indices;
            if (!gltf_indices) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }

            if (gltf_indices->component_type != cgltf_component_type_r_32u) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }
            if (gltf_indices->type != cgltf_type_scalar) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }
            if (gltf_indices->type != cgltf_type_scalar)
                if (!gltf_indices->buffer_view
                    || !gltf_indices->buffer_view->buffer
                    || !gltf_indices->buffer_view->buffer->data
                ) { return std::unexpected(AssetErrorCode::NotSupported); }
            if (!gltf_prim.attributes) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }

            const auto idx_buffer_view = gltf_indices->buffer_view;

            auto idx_data = read_buffer_data(
                static_cast<const u8*>(idx_buffer_view->buffer->data),
                gltf_indices->offset + idx_buffer_view->offset,
                gltf_indices->count,
                idx_buffer_view->stride,
                4,
                1
            );

            std::vector<Attribute> attr_list;
            std::vector<u8> positions;  // vec3
            std::vector<u8> normals;    // vec3
            std::vector<u8> tangents;   // vec3
            std::vector<u8> bitangents; // vec3
            std::vector<u8> textures;   // vec2
            std::vector<u8> colors;     // vec4
            u32 element_count = 0;
            for (u32 attr_idx = 0; attr_idx < gltf_prim.attributes_count; attr_idx++) {
                const auto& gltf_attribute      = gltf_prim.attributes[attr_idx];
                const auto attribute = gltf_attribute_to_siren(gltf_attribute.type);
                attr_list.push_back(attribute);
                const auto& prim_accessor    = gltf_attribute.data;
                const auto& prim_buffer_view = prim_accessor->buffer_view;
                const auto& prim_buffer      = prim_buffer_view->data;

                const auto src            = static_cast<const u8*>(prim_buffer);
                const auto offset         = prim_accessor->offset + prim_buffer_view->offset;
                const auto count          = prim_accessor->count;
                const auto stride         = prim_buffer_view->stride;
                const auto component_size = cgltf_component_size(prim_accessor->component_type);
                const auto num_components = cgltf_num_components(prim_accessor->type);
                element_count             = glm::max(element_count, (u32)prim_accessor->count);

                switch (attribute) {
                    case Attribute::Position: {
                        if (component_size != 4 || num_components != 3) {
                            return std::unexpected(AssetErrorCode::AssetCorrupted);
                        }
                        positions = read_buffer_data(src, offset, count, stride, component_size, num_components);
                        break;
                    }
                    case Attribute::Normal: {
                        if (component_size != 4 || num_components != 3) {
                            return std::unexpected(AssetErrorCode::AssetCorrupted);
                        }
                        normals = read_buffer_data(src, offset, count, stride, component_size, num_components);
                        break;
                    }
                    case Attribute::Tangent: {
                        if (component_size != 4 || num_components != 3) {
                            return std::unexpected(AssetErrorCode::AssetCorrupted);
                        }
                        tangents = read_buffer_data(src, offset, count, stride, component_size, num_components);
                        break;
                    }
                    case Attribute::Bitangent: {
                        if (component_size != 4 || num_components != 3) {
                            return std::unexpected(AssetErrorCode::AssetCorrupted);
                        }
                        bitangents = read_buffer_data(src, offset, count, stride, component_size, num_components);
                        break;
                    }
                    case Attribute::Texture: {
                        if (component_size != 4 || num_components != 2) {
                            return std::unexpected(AssetErrorCode::AssetCorrupted);
                        }
                        textures = read_buffer_data(src, offset, count, stride, component_size, num_components);
                        break;
                    }
                    case Attribute::Color: {
                        if (component_size != 4 || num_components != 4) {
                            return std::unexpected(AssetErrorCode::AssetCorrupted);
                        }
                        colors = read_buffer_data(src, offset, count, stride, component_size, num_components);
                        break;
                    }
                }
            }

            const auto get_element = []<typename T> (const std::vector<u8>& buf, const u32 idx) -> T{
                if (buf.empty()) { return T{ 0 }; }
                return reinterpret_cast<const T*>(buf.data())[idx];
            };

            ByteBuffer bytes;
            VertexBufferBuilder vbb{ VertexLayout{ std::move(attr_list) } };
            for (u32 elem_idx = 0; elem_idx < element_count; elem_idx++) {
                vbb.push_vertex(
                    {
                        .position = get_element.operator()<glm::vec3>(positions, elem_idx),
                        .normal = get_element.operator()<glm::vec3>(normals, elem_idx),
                        .tangent = get_element.operator()<glm::vec3>(tangents, elem_idx),
                        .bitangent = get_element.operator()<glm::vec3>(bitangents, elem_idx),
                        .texture = get_element.operator()<glm::vec2>(textures, elem_idx),
                        .color = get_element.operator()<glm::vec4>(colors, elem_idx),
                    }
                );
            }

            const auto& material_handle = materials[gltf_prim.material - data->materials];
            // todo: add name here
            auto index_buffer = ctx.device().create_buffer({
                .label = std::nullopt,
                .data = std::move(idx_data),
                .size = idx_data.size(),
                .usage = BufferUsage::Static,
            });

            BufferParams vb_params = vbb.build();
            const u32 data_size    = vb_params.data.size();
            // todo: add name here
            auto vertex_buffer = ctx.device().create_buffer({
                .label = std::nullopt,
                .data = std::move(vb_params.data),
                .size = data_size,
                .usage = BufferUsage::Static,
            });
            auto surface = std::make_unique<Surface>(
                material_handle,
                std::move(index_buffer),
                std::move(vertex_buffer),
                (u32)gltf_indices->count
            );
            const auto handle = ctx.add_labeled_asset(name, std::move(surface));
            mesh->surfaces.push_back(handle);
        }

        mesh->surfaces = std::move(surfaces);

        vec.emplace_back(ctx.add_labeled_asset(name, std::move(mesh)));
    }

    return vec;
}

static auto load_cameras(
    const cgltf_data* data
) -> std::expected<std::vector<SceneCamera>, AssetErrorCode> {
    NameIDGenerator name_gen{ .fallback = "Camera_" };

    std::vector<SceneCamera> vec;
    vec.reserve(data->cameras_count);

    for (u32 cam_idx = 0; cam_idx < data->cameras_count; cam_idx++) {
        const auto& gltf_camera = data->cameras[cam_idx];
        const auto name         = name_gen.next(gltf_camera.name);

        if (gltf_camera.type == cgltf_camera_type_perspective) {
            const auto& cam_data = gltf_camera.data.perspective;
            vec.emplace_back(
                SceneCamera{
                    .z_far = cam_data.has_zfar ? cam_data.zfar : 0,
                    .z_near = cam_data.zfar,
                    .perspective = {
                        .aspect_ratio = cam_data.has_aspect_ratio ? cam_data.aspect_ratio : 0,
                        .y_fov = cam_data.yfov,
                    },
                    .type = SceneCamera::Perspective,
                }
            );
        } else if (gltf_camera.type == cgltf_camera_type_orthographic) {
            const auto& cam_data = gltf_camera.data.orthographic;
            vec.emplace_back(
                SceneCamera{
                    .z_far = cam_data.zfar,
                    .z_near = cam_data.zfar,
                    .orthographic = {
                        .x_mag = cam_data.xmag,
                        .y_mag = cam_data.ymag,
                    },
                    .type = SceneCamera::Orthographic
                }
            );
        } else {
            return std::unexpected(AssetErrorCode::NotSupported);
        }
    }

    return vec;
}

static auto load_nodes(
    const cgltf_data* data,
    const std::vector<StrongHandle<Mesh>>& meshes,
    const std::vector<SceneCamera>& cameras,
    LoadContext& ctx
) -> std::expected<std::vector<StrongHandle<GltfNode>>, AssetErrorCode> {
    NameIDGenerator name_gen{ .fallback = "Node_" };

    const auto get_transform = [] (const cgltf_node& node)-> glm::mat4{
        f32 out[16];
        cgltf_node_transform_local(&node, (cgltf_float*)out);
        return glm::make_mat4(out);
    };

    const auto get_node_idx = [data] (const cgltf_node* node_ptr) -> usize{ return node_ptr - data->nodes; };

    std::vector<std::pair<StrongHandle<GltfNode>, GltfNode*>> vec;
    vec.reserve(data->nodes_count);

    // we do two passes for loading nodes.
    // the first pass instantiates shallow node objects without any relations to other nodes,
    // the second pass fills in any relations nodes may have

    for (usize node_idx = 0; node_idx < data->nodes_count; node_idx++) {
        const auto& gltf_node = data->nodes[node_idx];
        const auto name       = name_gen.next(gltf_node.name);

        std::optional<StrongHandle<Mesh>> mesh = std::nullopt;
        if (gltf_node.mesh != nullptr) {
            mesh = meshes[gltf_node.mesh - data->meshes];
        }

        std::optional<SceneCamera> camera = std::nullopt;
        if (gltf_node.camera != nullptr) {
            camera = cameras[gltf_node.camera - data->cameras];
        }

        auto node = std::make_unique<GltfNode>();

        node->name      = name;
        node->index     = get_node_idx(&gltf_node);
        node->transform = get_transform(gltf_node);
        node->parent    = std::nullopt;
        node->children  = { };
        node->mesh      = mesh;
        node->camera    = camera;

        // hacky ownership lol

        auto raw    = node.get();
        auto handle = ctx.add_labeled_asset(name, std::move(node));
        vec.emplace_back(handle, raw);
    }

    for (usize node_idx = 0; node_idx < data->nodes_count; node_idx++) {
        const auto& gltf_node = data->nodes[node_idx];
        auto& [handle, node]  = vec[node_idx];

        if (gltf_node.parent) {
            const auto parent_idx = get_node_idx(gltf_node.parent);
            auto& parent_handle   = vec[parent_idx].first;
            node->parent          = parent_handle.as_weak();
        }

        std::vector<StrongHandle<GltfNode>> children;
        for (usize child_idx = 0; child_idx < gltf_node.children_count; child_idx++) {
            auto& child_handle = vec[get_node_idx(gltf_node.children[child_idx])].first;
            children.emplace_back(child_handle);
        }

        node->children = std::move(children);
    }

    return vec | views::keys | ranges::to<std::vector>();
}

static auto load_scenes(
    const cgltf_data* data,
    const std::vector<StrongHandle<GltfNode>>& nodes,
    LoadContext& ctx
) -> std::expected<std::vector<StrongHandle<GltfScene>>, AssetErrorCode> {
    const auto create_name = [] (const char* name) -> std::string{
        static u32 count = 0;
        if (name) { return name; }
        return "Node_" + std::to_string(count);
    };

    std::vector<StrongHandle<GltfScene>> vec;
    vec.reserve(data->scenes_count);

    for (usize scene_idx = 0; scene_idx < data->scenes_count; scene_idx++) {
        const auto& gltf_scene = data->scenes[scene_idx];
        const auto name        = create_name(gltf_scene.name);
        std::vector<StrongHandle<GltfNode>> root_nodes;

        for (usize node_idx = 0; node_idx < gltf_scene.nodes_count; node_idx++) {
            const auto gltf_node       = gltf_scene.nodes[node_idx];
            const auto global_node_idx = gltf_node - data->nodes;
            root_nodes.emplace_back(nodes[global_node_idx]);
        }

        auto scene = std::make_unique<GltfScene>(
            name,
            scene_idx - data->scenes_count,
            std::move(root_nodes)
        );

        const auto handle = ctx.add_labeled_asset(gltf_scene.name, std::move(scene));

        vec.emplace_back(handle);
    }

    return vec;
}

auto GltfLoader::load(LoadContext&& ctx, const LoaderConfig& config) const -> std::expected<void, Error> {
    if (!std::holds_alternative<GltfLoaderConfig>(config)) { return std::unexpected(Error(Code::LogicFail)); }
    const auto config_ = std::get<GltfLoaderConfig>(config);

    // @formatter:off
    struct cgltf_delete { auto operator()(cgltf_data* data) const -> void { cgltf_free(data); } };
    using cgltf_ptr = std::unique_ptr<cgltf_data, cgltf_delete>;
    // @formatter:on

    const auto data = FileSystem::to_physical(ctx.path().path()).and_then(
        [] (const Path& p) -> std::optional<cgltf_ptr>{
            cgltf_data* data = nullptr;
            constexpr cgltf_options options{ };

            if (cgltf_parse_file(&options, p.string().c_str(), &data) != cgltf_result_success) { return std::nullopt; }

            if (cgltf_validate(data) != cgltf_result_success) {
                cgltf_free(data);
                return std::nullopt;
            }

            if (cgltf_load_buffers(&options, data, p.string().c_str()) != cgltf_result_success) {
                cgltf_free(data);
                return std::nullopt;
            }

            return cgltf_ptr(data);
        }
    ).value_or(nullptr);

    if (!data) { return std::unexpected(Error(Code::AssetCorrupted)); }

    const auto textures_ = load_textures(data.get(), ctx);
    if (!textures_.has_value()) { return std::unexpected(textures_.error()); }
    std::vector<StrongHandle<Texture>> textures = textures_.value();

    const auto materials_ = load_materials(data.get(), textures, ctx);
    if (!materials_.has_value()) { return std::unexpected(materials_.error()); }
    std::vector<StrongHandle<PBRMaterial>> materials = materials_.value();

    const auto meshes_ = load_meshes(data.get(), materials, ctx);
    if (!meshes_.has_value()) { return std::unexpected(meshes_.error()); }
    std::vector<StrongHandle<Mesh>> meshes = meshes_.value();

    const auto cameras_ = load_cameras(data.get());
    if (!cameras_.has_value()) { return std::unexpected(cameras_.error()); }
    std::vector<SceneCamera> cameras = cameras_.value();

    const auto nodes_ = load_nodes(data.get(), meshes, cameras, ctx);
    if (!nodes_.has_value()) { return std::unexpected(nodes_.error()); }
    std::vector<StrongHandle<GltfNode>> nodes = nodes_.value();

    const auto scenes_result = load_scenes(data.get(), nodes, ctx);
    if (!scenes_result.has_value()) { return std::unexpected(scenes_result.error()); }
    std::vector<StrongHandle<GltfScene>> scenes = scenes_result.value();

    std::optional<StrongHandle<GltfScene>> default_scene;
    if (data->scene) { default_scene = scenes[data->scene - data->scenes]; }

    auto gltf = std::make_unique<Gltf>(
        std::move(scenes),
        std::move(default_scene),
        std::move(meshes),
        std::move(materials),
        std::move(nodes),
        std::move(cameras)
    );

    ctx.finish(std::move(gltf));

    return { };
}

} // namespace siren
