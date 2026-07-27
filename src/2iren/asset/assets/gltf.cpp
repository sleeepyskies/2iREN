#include "gltf.hpp"

#include <expected>
#include <glm/gtc/integer.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stb_image.h>
#include "2iren/asset/asset_server.hpp"
#include "2iren/asset/asset_utils.hpp"
#include "2iren/rhi/device.hpp"
#include "2iren/util/cgltf.cpp"
#include "2iren/util/filesystem.hpp"

/// For docs on GLTF see: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#indices-and-names
/// For a brief overview of GLTF see: https://www.khronos.org/files/gltf20-reference-guide.pdf

// todo:
//      we do a mix of assertions and error handling here, but not consitently.
//      i should at some point go through code and make consistent.

struct NameIDGenerator {
    std::string fallback = "Unnamed";
    siren::u32 count     = 0;
    auto next(const char* name) -> std::string {
        if (name) {
            return name;
        }
        return fallback + "_" + std::to_string(count++);
    }
    auto next() -> std::string { return next(nullptr); }
};

namespace siren {
// ============================================================================
// == MARK: Mappings
// ============================================================================

[[maybe_unused]]
static auto gltf_attribute_to_siren(const cgltf_attribute_type attribute) -> Attribute {
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

[[maybe_unused]]
static auto gltf_index_type_to_siren(const cgltf_component_type type) -> IndexFormat {
    switch (type) {
        case cgltf_component_type_r_8u: return IndexFormat::UInt8;
        case cgltf_component_type_r_16u: return IndexFormat::UInt16;
        case cgltf_component_type_r_32u: return IndexFormat::UInt32;

        case cgltf_component_type_r_8:
        case cgltf_component_type_r_16:
        case cgltf_component_type_r_32f: PANIC("Attempted to define either a Int8, Int16 or Float32 as an index type.");

        case cgltf_component_type_max_enum:
        case cgltf_component_type_invalid:
        default: UNREACHABLE("Could not convert cgltf_component_type to 2iren DataType");
    }
}

[[maybe_unused]]
static auto gltf_type_to_siren(const cgltf_component_type type) -> DataType {
    switch (type) {
        case cgltf_component_type_r_8: return DataType::Int8;
        case cgltf_component_type_r_8u: return DataType::UInt8;
        case cgltf_component_type_r_16: return DataType::Int16;
        case cgltf_component_type_r_16u: return DataType::UInt16;
        case cgltf_component_type_r_32u: return DataType::UInt32;
        case cgltf_component_type_r_32f: return DataType::Float32;

        case cgltf_component_type_max_enum:
        case cgltf_component_type_invalid:
        default: UNREACHABLE("Could not convert cgltf_component_type to 2iren DataType");
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

static auto parse_sampler(const cgltf_sampler* sampler, Device& device) -> Sampler {
    // use default sampler if none is provided.
    SamplerDescriptor desc;
    if (sampler) {
        desc.min_filter    = gltf_filter_to_siren(sampler->min_filter);
        desc.max_filter    = gltf_filter_to_siren(sampler->mag_filter);
        desc.mipmap_filter = gltf_mipmap_filter_to_siren(sampler->min_filter);
        desc.s_wrap        = gltf_wrap_to_siren(sampler->wrap_s);
        desc.t_wrap        = gltf_wrap_to_siren(sampler->wrap_t);
        // the following are not provided by gltf spec:
        // r_wrap, lod_min, lod_max, border_color, compare_mode, compare_fn
    }
    return device.create_sampler(std::move(desc));
}

static auto load_textures(const cgltf_data* data, LoadContext& ctx)
    -> std::expected<std::vector<StrongHandle<Texture>>, AssetErrorCode> {
    NameIDGenerator name_gen{.fallback = "Texture"};

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
            if (!path) {
                return std::unexpected(AssetErrorCode::FileNotFound);
            }
            handle = ctx.load_external_asset<Texture>(path.value().string(),
                std::make_optional(TextureLoader::ConfigType{
                    .name                   = name,
                    .format                 = ImageFormat::Unknown, // trust the texture loader can handle lmao
                    .sampler                = std::move(sampler),
                    .generate_mipmap_levels = true,
                }));
        } else if (texture.image->buffer_view) {
            // load image from buffer
            // todo: we dont actually upload the image data here?
            if (texture.image->buffer_view->has_meshopt_compression) {
                return std::unexpected(AssetErrorCode::NotSupported);
            }
            const u8* bytes = cgltf_buffer_view_data(texture.image->buffer_view);
            const auto size = texture.image->buffer_view->size;

            i32 width, height, channels;
            std::unique_ptr<u8, void (*)(void*)> img_data(
                stbi_load_from_memory(bytes, (int)size, &width, &height, &channels, STBI_default), stbi_image_free);
            if (!img_data) {
                return std::unexpected(AssetErrorCode::AssetCorrupted);
            }
            const usize img_data_size = width * height * channels;

            // todo: put into a function, also this might not be enough? how do we know if 3 channels is rgb or srgb?
            const auto format = channels == 1 ? ImageFormat::R8
                : channels == 3               ? ImageFormat::RGB8
                : channels == 4               ? ImageFormat::RGBA8
                                              : ImageFormat::Unknown;

            const auto extent = ImageExtent{.width = (u32)width, .height = (u32)height, .depth_or_layers = 1};

            const u32 max_dim       = std::max({extent.width, extent.height, extent.depth_or_layers});
            const u32 mipmap_levels = 1 + static_cast<u32>(glm::floor(glm::log2(max_dim)));

            // todo: add name?
            auto img      = ctx.device().create_image({
                     .label         = std::nullopt,
                     .format        = format,
                     .extent        = extent,
                     .dimension     = ImageDimension::D2,
                     .mipmap_levels = mipmap_levels,
            });
            auto resource = ctx.device().record_resource_commands();
            resource.upload_to_image(img.handle(), std::span(img_data.get(), img_data_size));
            ctx.device().submit(resource.finish());
            handle = ctx.add_labeled_asset<Texture>(
                name, std::make_unique<Texture>(name, std::move(img), std::move(sampler)));
        } else {
            return std::unexpected(AssetErrorCode::AssetCorrupted);
        }

        if (!handle.is_valid()) {
            return std::unexpected(AssetErrorCode::InvalidFormat);
        }
        vec.emplace_back(std::move(handle));
    }

    return vec;
}

static auto load_materials(const cgltf_data* data, const std::vector<StrongHandle<Texture>>& textures, LoadContext& ctx)
    -> std::expected<std::vector<StrongHandle<MaterialAsset>>, AssetErrorCode> {
    NameIDGenerator name_gen{.fallback = "Material"};

    const auto get_texture = [&textures, &data](
                                 const cgltf_texture* texture) -> std::expected<StrongHandle<Texture>, AssetErrorCode> {
        const usize idx = texture - data->textures;
        if (idx >= textures.size()) {
            return std::unexpected(AssetErrorCode::AssetCorrupted);
        }
        return textures[idx];
    };

    std::vector<StrongHandle<MaterialAsset>> vec;
    vec.reserve(data->materials_count);

    for (usize material_idx = 0; material_idx < data->materials_count; material_idx++) {
        const auto& gltf_material = data->materials[material_idx];
        const auto name           = name_gen.next(gltf_material.name);
        auto mat                  = std::make_unique<MaterialAsset>(name);

        // metallic roughness and specular glossiness are mutually exclusive. we stick to just metallic roughness

        if (gltf_material.has_pbr_metallic_roughness) {
            const auto& pbr_mr = gltf_material.pbr_metallic_roughness;
            mat->set_base_color(Rgba{glm::make_vec4(pbr_mr.base_color_factor)});
            mat->set_metallic(pbr_mr.metallic_factor);
            mat->set_roughness(pbr_mr.roughness_factor);
            if (pbr_mr.base_color_texture.texture) {
                const auto texture = get_texture(pbr_mr.base_color_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_base_color_tex(texture.value());
            }
            if (pbr_mr.metallic_roughness_texture.texture) {
                const auto texture = get_texture(pbr_mr.metallic_roughness_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_metallic_roughness_tex(texture.value());
            }
        }

        if (gltf_material.has_pbr_specular_glossiness) { /* pass */
        }

        if (gltf_material.has_clearcoat) {
            const auto& cc = gltf_material.clearcoat;
            mat->set_clear_coat(cc.clearcoat_factor);
            mat->set_clear_coat_roughness(cc.clearcoat_roughness_factor);
            if (cc.clearcoat_texture.texture) {
                const auto texture = get_texture(cc.clearcoat_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_clear_coat_tex(texture.value());
            }
            if (cc.clearcoat_roughness_texture.texture) {
                const auto texture = get_texture(cc.clearcoat_roughness_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_clear_coat_tex(texture.value());
            }
            if (cc.clearcoat_normal_texture.texture) {
                const auto texture = get_texture(cc.clearcoat_normal_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_clear_coat_tex(texture.value());
            }
        }

        if (gltf_material.has_transmission) {
            const auto& tr = gltf_material.transmission;
            mat->set_transmission(tr.transmission_factor);
            if (tr.transmission_texture.texture) {
                const auto texture = get_texture(tr.transmission_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
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
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
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
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_specular_color_tex(texture.value());
            }
            if (spec.specular_texture.texture) {
                const auto texture = get_texture(spec.specular_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_specular_tex(texture.value());
            }
        }

        if (gltf_material.has_sheen) {
            const auto& sh = gltf_material.sheen;
            mat->set_sheen_color(glm::make_vec3(sh.sheen_color_factor));
            mat->set_sheen_roughness(sh.sheen_roughness_factor);
            if (sh.sheen_color_texture.texture) {
                const auto texture = get_texture(sh.sheen_color_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_sheen_color_tex(texture.value());
            }
            if (sh.sheen_roughness_texture.texture) {
                const auto texture = get_texture(sh.sheen_roughness_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
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
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_iridescence_tex(texture.value());
            }
            if (ir.iridescence_thickness_texture.texture) {
                const auto texture = get_texture(ir.iridescence_thickness_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_iridescence_thickness_tex(texture.value());
            }
        }

        if (gltf_material.has_diffuse_transmission) {
            const auto& df = gltf_material.diffuse_transmission;
            mat->set_diffuse_transmission_factor(df.diffuse_transmission_factor);
            mat->set_diffuse_transmission_color(glm::make_vec3(df.diffuse_transmission_color_factor));
            if (df.diffuse_transmission_texture.texture) {
                const auto texture = get_texture(df.diffuse_transmission_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_diffuse_transmission_tex(texture.value());
            }
            if (df.diffuse_transmission_color_texture.texture) {
                const auto texture = get_texture(df.diffuse_transmission_color_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_diffuse_transmission_color_tex(texture.value());
            }
        }

        if (gltf_material.has_anisotropy) {
            const auto& an = gltf_material.anisotropy;
            mat->set_anisotropy_strength(an.anisotropy_strength);
            mat->set_anisotropy_rotation(an.anisotropy_rotation);
            if (an.anisotropy_texture.texture) {
                const auto texture = get_texture(an.anisotropy_texture.texture);
                if (!texture.has_value()) {
                    return std::unexpected(texture.error());
                }
                mat->set_anisotropy_tex(texture.value());
            }
        }

        if (gltf_material.has_dispersion) {
            mat->set_dispersion(gltf_material.dispersion.dispersion);
        }

        if (gltf_material.normal_texture.texture) {
            const auto texture = get_texture(gltf_material.normal_texture.texture);
            if (!texture.has_value()) {
                return std::unexpected(texture.error());
            }
            mat->set_normal_tex(texture.value());
        }

        if (gltf_material.occlusion_texture.texture) {
            const auto texture = get_texture(gltf_material.occlusion_texture.texture);
            if (!texture.has_value()) {
                return std::unexpected(texture.error());
            }
            mat->set_occlusion_tex(texture.value());
        }

        if (gltf_material.emissive_texture.texture) {
            const auto texture = get_texture(gltf_material.emissive_texture.texture);
            if (!texture.has_value()) {
                return std::unexpected(texture.error());
            }
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

static auto check_gltf_primitive(const cgltf_primitive& primitve) -> AssetLoadError {
    if (primitve.type != cgltf_primitive_type_triangles) {
        log::warn(
            "2iren only supports primitive type triangles. Encountered cgltf_primitive_type: {}", (u32)primitve.type);
        return std::unexpected(AssetErrorCode::NotSupported);
    }

    if (primitve.has_draco_mesh_compression) {
        log::warn("2iren does not support draco gltf mesh compression.");
        return std::unexpected(AssetErrorCode::NotSupported);
    }

    if (!primitve.attributes) {
        log::error("gltf primitive contains no vertex attributes.");
        return std::unexpected(AssetErrorCode::AssetCorrupted);
    }

    return {};
}

static auto validate_index_type(const cgltf_component_type type) -> AssetLoadError {
    if (type == cgltf_component_type_r_8u) {
        log::trace("Surface has index type is UInt8, will be converted to UInt32.");
    } else if (type == cgltf_component_type_r_16u) {
        log::trace("Surface has index type is UInt16, will be converted to UInt32.");
    } else if (type == cgltf_component_type_r_32u) {
        // do nothing, log nothing is fine
    } else {
        log::error("Invalid glTF index type encountered, cgltf_component_type value: {}.", (u32)type);
        return std::unexpected(AssetErrorCode::AssetCorrupted);
    }

    return {};
}

static auto validate_gltf_indices(const cgltf_accessor* indices) -> AssetLoadError {
    if (!indices) {
        log::warn("2iren does not support gltf meshes without indices.");
        return std::unexpected(AssetErrorCode::NotSupported);
    }

    if (const auto res = validate_index_type(indices->component_type); !res) {
        return res;
    }

    if (indices->type != cgltf_type_scalar) {
        log::warn("gltf mesh has non scalar indices.");
        return std::unexpected(AssetErrorCode::AssetCorrupted);
    }

    if (!indices->buffer_view || !indices->buffer_view->buffer || !indices->buffer_view->buffer->data) {
        log::error("gltf mesh indices point to missing or invalid buffer data.");
        return std::unexpected(AssetErrorCode::AssetCorrupted);
    }

    return {};
}

static auto load_index_buffer(const cgltf_accessor* indices, Device& device)
    -> std::expected<IndexBuffer, AssetErrorCode> {
    // not that 2iren only supports 32-bit unsinged integers for now as indices.
    // at the end of the day, this probably doesnt matter too much, but it would b nice to maybe
    // support lower bit indices?

    if (const auto res = validate_gltf_indices(indices); !res) {
        return std::unexpected(res.error());
    }

    const auto index_count = indices->count;

    // have to resize not just reserve since a c-style api wants direct buffer access
    ByteBuffer buffer;
    buffer.data().resize(index_count * sizeof(u32));

    const usize unpacked_count = cgltf_accessor_unpack_indices(indices, buffer.raw(), sizeof(u32), index_count);
    ASSERT(index_count == unpacked_count, "Number of parsed indices did not match original accessor index count.");

    return IndexBuffer{
        .buffer = device.create_buffer({
            .label = "Index Buffer",
            .data  = buffer.data(), // todo: this does a copy lol, maybe we should accept a ByteBuffer instead?
            .size  = buffer.size_bytes(),
            .usage = BufferUsage::Static,
        }),
        .count  = index_count,
        .format = IndexFormat::UInt32,
    };
}

static auto load_vertex_layout(const cgltf_primitive&) -> Layout {
    return DEFAULT_VERTEX_LAYOUT;

    // code below returns the actual layout
    /*
    auto layout_builder = LayoutBuilder::start();
    for (usize i = 0; i < primitive.attributes_count; i++) {
        const auto& gltf_attribute = primitive.attributes[i];
        const auto& accessor       = gltf_attribute.data;

        layout_builder.add(
            gltf_attribute_to_siren(gltf_attribute.type),
            cgltf_num_components(accessor->type),
            gltf_type_to_siren(accessor->component_type)
        );
    }
    return layout_builder.finish();
    */
}

static auto load_vertex_buffer(const cgltf_primitive& primitive, Device& device) -> VertexBuffer {
    // when loading a vertex buffer, we enforce that each attribute exists. This means
    // we write dummy data into the vertex buffer for any attributes missing in the actual
    // underlying gltf buffers. Eventually, shader permuations would be nice, but for now we do this.

    // first, load layout
    const auto layout = load_vertex_layout(primitive);

    // then validate attribute size
    const usize count = primitive.attributes[0].data->count;
    for (usize i = 0; i < primitive.attributes_count; i++) {
        ASSERT(count == primitive.attributes[i].data->count, "gltf vertex attributes must all have the same count!");
    }

    ByteBuffer buffer;
    buffer.reserve_bytes(layout.stride * count); // size of a single vertex * number of vertices

    // get all accessors
    cgltf_accessor* positions = nullptr;
    cgltf_accessor* normals   = nullptr;
    cgltf_accessor* colors    = nullptr;
    cgltf_accessor* textures  = nullptr;
    cgltf_accessor* tangents  = nullptr;

    for (usize i = 0; i < primitive.attributes_count; i++) {
        const auto& attribute = primitive.attributes[i];
        switch (const auto type = attribute.type) {
            case cgltf_attribute_type_position: positions = attribute.data; break;
            case cgltf_attribute_type_normal: normals = attribute.data; break;
            case cgltf_attribute_type_color: colors = attribute.data; break;
            case cgltf_attribute_type_texcoord: textures = attribute.data; break;
            case cgltf_attribute_type_tangent: tangents = attribute.data; break;

            case cgltf_attribute_type_joints:
            case cgltf_attribute_type_weights:
            case cgltf_attribute_type_custom: {
                log::warn("gltf attribute contains unsupported cgltf_attribute_type: {}, skipping this.", (usize)type);
                break;
            }

            case cgltf_attribute_type_invalid:
            case cgltf_attribute_type_max_enum: PANIC("gltf file contains invalid attribute type.");
        }
    }

    ASSERT(positions, "Surface does not contain a positional attribute!");

    for (usize i = 0; i < count; i++) {
        std::array<f32, 4> position = {0.f, 0.f, 0.f, 1.f};
        cgltf_accessor_read_float(positions, i, (cgltf_float*)position.data(), 3);
        buffer.append(position);

        std::array<f32, 4> normal = {0.f, 1.f, 0.f, 0.f};
        if (normals) {
            cgltf_accessor_read_float(normals, i, (cgltf_float*)normal.data(), 3);
        }
        buffer.append(normal);

        std::array<f32, 4> color = {1.f, 1.f, 1.f, 1.f};
        if (colors) {
            cgltf_accessor_read_float(colors, i, (cgltf_float*)color.data(), 4);
        }
        buffer.append(color);

        std::array<f32, 2> texture = {0.f, 0.f};
        if (textures) {
            cgltf_accessor_read_float(textures, i, (cgltf_float*)texture.data(), 2);
        }
        buffer.append(texture);

        std::array<f32, 4> tangent = {1.f, 0.f, 0.f, 1.f};
        if (tangents) {
            cgltf_accessor_read_float(tangents, i, (cgltf_float*)tangent.data(), 3);
        }
        buffer.append(tangent);
    }

    return VertexBuffer{
        .buffer = device.create_buffer({
            .label = "Vertex Buffer",
            .data  = buffer.data(), // todo: also does a copy here fuck
            .size  = buffer.size_bytes(),
            .usage = BufferUsage::Static,
        }),
        .layout = layout,
    };
}

static auto load_meshes(
    const cgltf_data* data, const std::vector<StrongHandle<MaterialAsset>>& materials, LoadContext& ctx)
    -> std::expected<std::vector<StrongHandle<Mesh>>, AssetErrorCode> {
    // surface names scoped are scoped to the gltf due to asset label system.
    NameIDGenerator mesh_name_generator{.fallback = "Mesh"};
    NameIDGenerator surface_name_generator{.fallback = "Surface"};

    std::vector<StrongHandle<Mesh>> vec;
    vec.reserve(data->meshes_count);

    // iterate over each mesh inside the file
    for (usize mesh_idx = 0; mesh_idx < data->meshes_count; mesh_idx++) {
        const auto& gltf_mesh = data->meshes[mesh_idx];

        auto mesh  = std::make_unique<Mesh>();
        mesh->name = mesh_name_generator.next(gltf_mesh.name);
        mesh->surfaces.reserve(gltf_mesh.primitives_count);

        // iterate each surface of the individual meshes
        for (usize prim_idx = 0; prim_idx < gltf_mesh.primitives_count; prim_idx++) {
            // get the relevant primitive, aka the relevant Surface. then validate
            const auto& gltf_prim = gltf_mesh.primitives[prim_idx];
            if (const auto res = check_gltf_primitive(gltf_prim); !res) {
                return std::unexpected(res.error());
            }

            // load and validate index buffer
            auto index_buffer = load_index_buffer(gltf_prim.indices, ctx.device());
            if (!index_buffer) {
                return std::unexpected(index_buffer.error());
            }

            // load vertex data according to layout
            auto vertex_buffer = load_vertex_buffer(gltf_prim, ctx.device());

            const auto& material_handle = (gltf_prim.material == nullptr)
                ? ctx.fetch_default<MaterialAsset>()
                : materials[gltf_prim.material - data->materials];

            auto surface = std::make_unique<Surface>(
                surface_name_generator.next(), material_handle, std::move(*index_buffer), std::move(vertex_buffer));

            mesh->surfaces.emplace_back(ctx.add_labeled_asset(surface->name, std::move(surface)));
        }

        vec.emplace_back(ctx.add_labeled_asset(mesh->name, std::move(mesh)));
    }

    return vec;
}

static auto load_nodes(const cgltf_data* data, const std::vector<StrongHandle<Mesh>>& meshes, LoadContext& ctx)
    -> std::expected<std::vector<StrongHandle<GltfNode>>, AssetErrorCode> {
    NameIDGenerator name_gen{.fallback = "Node_"};

    const auto get_transform = [](const cgltf_node& node) -> glm::mat4 {
        glm::mat4 transform;
        cgltf_node_transform_local(&node, glm::value_ptr(transform));
        return transform;
    };

    const auto get_node_idx = [data](const cgltf_node* node_ptr) -> usize { return node_ptr - data->nodes; };

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

        auto node = std::make_unique<GltfNode>();

        node->name      = name;
        node->index     = get_node_idx(&gltf_node);
        node->transform = get_transform(gltf_node);
        node->parent    = std::nullopt;
        node->children  = {};
        node->mesh      = mesh;

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
            node->parent          = make_weak(parent_handle);
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

static auto load_scenes(const cgltf_data* data, const std::vector<StrongHandle<GltfNode>>& nodes, LoadContext& ctx)
    -> std::expected<std::vector<StrongHandle<GltfScene>>, AssetErrorCode> {
    NameIDGenerator name_gen{.fallback = "Node_"};

    std::vector<StrongHandle<GltfScene>> vec;
    vec.reserve(data->scenes_count);

    for (usize scene_idx = 0; scene_idx < data->scenes_count; scene_idx++) {
        const auto& gltf_scene = data->scenes[scene_idx];
        const auto name        = name_gen.next(gltf_scene.name);
        std::vector<StrongHandle<GltfNode>> root_nodes;

        for (usize node_idx = 0; node_idx < gltf_scene.nodes_count; node_idx++) {
            const auto gltf_node       = gltf_scene.nodes[node_idx];
            const auto global_node_idx = gltf_node - data->nodes;
            root_nodes.emplace_back(nodes[global_node_idx]);
        }

        auto scene = std::make_unique<GltfScene>(name, scene_idx - data->scenes_count, std::move(root_nodes));

        const auto handle = ctx.add_labeled_asset(name, std::move(scene));

        vec.emplace_back(handle);
    }

    return vec;
}

auto GltfLoader::load(LoadContext&& ctx, std::optional<ConfigType>) const -> AssetLoadError {
    log::debug("Loading a new gltf file from {}", ctx.path());

    // const auto config_ = config.value_or(ConfigType{ });

    // @formatter:off
    struct cgltf_delete {
        auto operator()(cgltf_data* data) const -> void { cgltf_free(data); }
    };
    using cgltf_ptr = std::unique_ptr<cgltf_data, cgltf_delete>;
    // @formatter:on

    // load the gltf file using cgltf
    auto physical_path_opt = FileSystem::to_physical(ctx.path().full_path());
    cgltf_data* raw        = nullptr;

    if (physical_path_opt) {
        const Path& p = *physical_path_opt;
        cgltf_options options{};

        if (cgltf_parse_file(&options, p.string().c_str(), &raw) != cgltf_result_success) {
            log::warn("Could not parse gltf at {}", p.string());
        } else if (cgltf_validate(raw) != cgltf_result_success) {
            log::warn("Could not validate gltf at {}", p.string());
            cgltf_free(raw);
            raw = nullptr;
        } else if (cgltf_load_buffers(&options, raw, p.string().c_str()) != cgltf_result_success) {
            log::warn("Could not load gltf buffers at {}", p.string());
            cgltf_free(raw);
            raw = nullptr;
        }
    }

    const auto data = cgltf_ptr(raw);

    if (!data) {
        log::warn("Could not load gltf at {}, vfs path does not exist.", ctx.path());
        return std::unexpected(AssetErrorCode::AssetCorrupted);
    }

    const auto textures = load_textures(data.get(), ctx);
    if (!textures.has_value()) {
        log::warn("Could not load gltf at {}, textures could not be loaded.", ctx.path());
        return std::unexpected(textures.error());
    }

    auto materials = load_materials(data.get(), *textures, ctx);
    if (!materials.has_value()) {
        log::warn("Could not load gltf at {}, materials could not be loaded.", ctx.path());
        return std::unexpected(materials.error());
    }

    auto meshes = load_meshes(data.get(), *materials, ctx);
    if (!meshes.has_value()) {
        log::warn("Could not load gltf at {}, meshes could not be loaded.", ctx.path());
        return std::unexpected(meshes.error());
    }

    auto nodes = load_nodes(data.get(), *meshes, ctx);
    if (!nodes.has_value()) {
        log::warn("Could not load gltf at {}, nodes could not be loaded.", ctx.path());
        return std::unexpected(nodes.error());
    }

    auto scenes = load_scenes(data.get(), *nodes, ctx);
    if (!scenes.has_value()) {
        log::warn("Could not load gltf at {}, scenes could not be loaded.", ctx.path());
        return std::unexpected(scenes.error());
    }

    std::optional<StrongHandle<GltfScene>> default_scene;
    if (data->scene) {
        default_scene = (*scenes)[data->scene - data->scenes];
    }

    ctx.finish(std::make_unique<Gltf>(
        std::move(*scenes), std::move(default_scene), std::move(*meshes), std::move(*materials), std::move(*nodes)));

    log::debug("gltf file successfully loaded into asset {}", ctx.handle());

    return {};
}

} // namespace siren
