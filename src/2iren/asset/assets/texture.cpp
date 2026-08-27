#include "texture.hpp"

#include <algorithm>
#include "2iren/util/stb_image.h"
#include <yaml-cpp/yaml.h>

#include "2iren/asset/asset_server.hpp"
#include "2iren/base.hpp"
#include "2iren/rhi/device.hpp"
#include "2iren/util/filesystem.hpp"
#include "2iren/util/log.hpp"

namespace siren {
namespace filetypes {
    static const std::vector<std::string> SRGB = {"png", "jpg", "jpeg"};
    static const std::vector<std::string> HDR  = {"exr", "hdr"};
} // namespace filetypes

// basically just string constants to avoid typos etc
namespace keys {
    constexpr std::string_view NAME = "name";

    constexpr std::string NX = "nx";
    constexpr std::string NY = "ny";
    constexpr std::string NZ = "nz";
    constexpr std::string PX = "px";
    constexpr std::string PY = "py";
    constexpr std::string PZ = "pz";
} // namespace keys

// todo: this only loads 2d images

static auto fetch_optional(const YAML::Node& node, const std::string_view key) -> std::optional<std::string> {
    return node[key] ? std::make_optional(node[key].as<std::string>()) : std::nullopt;
}

static auto invalid_schema(const std::string_view msg) -> AssetLoadError {
    log::warn("Invalid Schema found: {}", msg);
    return std::unexpected(AssetErrorCode::InvalidSchema);
}

[[nodiscard]] static auto calc_mipmap_levels(const u32, const u32) -> u32 {
    // todo: mipmap levels are never generated atm
    return 1;
}

[[nodiscard]] static auto no_config() -> AssetLoadError {
    log::warn("No config was provided, cannot proceed with loading texture.");
    return std::unexpected(AssetErrorCode::NoConfig);
}

[[nodiscard]] static auto file_not_found(const AssetPath& path) -> AssetLoadError {
    log::warn("File could not be found at: {}", path);
    return std::unexpected(AssetErrorCode::FileNotFound);
}

[[nodiscard]] static auto file_not_found(const std::string_view path) -> AssetLoadError {
    log::warn("File not found at path: {}", path);
    return std::unexpected(AssetErrorCode::FileNotFound);
}

[[nodiscard]] static auto invalid_format(
    const std::string_view path,
    const std::string_view msg = ""
) -> AssetLoadError {
    log::warn("Invalid YAML syntax in cubmap file: {}. Message: {}", path, msg);
    return std::unexpected(AssetErrorCode::InvalidFormat);
}

[[nodiscard]] static auto determine_format(
    const TextureLoader::ConfigType& cfg,
    const std::string& ext
)
    -> ImageFormat {
    if (cfg.format) {
        return *cfg.format;
    }

    if (std::ranges::contains(filetypes::SRGB, ext)) {
        log::trace("Guessing extension {} image has format LinearColor8.", ext);
        return ImageFormat::RGBA8;
    }

    if (std::ranges::contains(filetypes::HDR, ext)) {
        log::trace("Guessing extension {} image has format Hdr16.", ext);
        return ImageFormat::RGB16f;
    }

    log::trace("Could nopt guess image format.");
    return ImageFormat::Unknown;
}

[[maybe_unused]] [[nodiscard]] static auto determine_srgb(const ImageFormat format) -> bool {
    switch (format) {
        case ImageFormat::sRGB8:
        case ImageFormat::sRGBA8: return true;

        case ImageFormat::R8:
        case ImageFormat::RGB8:
        case ImageFormat::RGBA8:
        case ImageFormat::RGB16f:
        case ImageFormat::RG32f:
        case ImageFormat::Depth24Stencil8:
        case ImageFormat::Unknown: return false;

        default: UNREACHABLE("Could not determine srgb from ImageFormat");
    }
}

auto TextureLoader::load(LoadContext&& ctx, std::optional<ConfigType> config) const -> AssetLoadError {
    stbi_set_flip_vertically_on_load(true);
    if (!config) {
        return no_config();
    }

    const auto path = FileSystem::to_physical(ctx.path().full_path());
    if (!path) {
        return file_not_found(ctx.path());
    }

    if (ctx.path().extension() == "cubemap") {
        // special branch for cubemaps
        return load_cubemap(std::move(ctx), std::move(*config), *path);
    }

    const auto tname = config->name.value_or(ctx.path().filename());
    const auto iname = std::format("{}_Image", tname);

    const auto format = determine_format(*config, ctx.path().extension());
    // const auto is_srgb = determine_srgb(format);

    i32 width                = 0, height = 0, channels = 0;
    u8* data                 = stbi_load(path->c_str(), &width, &height, &channels, 0);
    const ImageExtent extent = {
        .width           = static_cast<u32>(width),
        .height          = static_cast<u32>(height),
        .depth_or_layers = 1,
    };
    const u32 mipmap_levels = config->generate_mipmap_levels ? calc_mipmap_levels(width, height) : 1;
    if (!data) {
        log::warn("Could not load, reason: {}", stbi_failure_reason());
    }
    const usize data_size = width * height * channels;

    auto image = ctx.device().create_image(
        {
            .label         = iname,
            .format        = format,
            .extent        = extent,
            .dimension     = ImageDimension::D2,
            .mipmap_levels = mipmap_levels,
        }
    );
    auto resource = ctx.device().record_resource_commands();
    resource.upload_to_image(image.handle(), std::span(data, data_size));
    ctx.device().submit(resource.finish());

    stbi_image_free(data);
    ctx.finish(std::make_unique<Texture>(tname, std::move(image), std::move(config->sampler)));

    return {};
}

auto TextureLoader::load_cubemap(
    LoadContext&& ctx,
    ConfigType&& config,
    const Path path
) const -> AssetLoadError {
    stbi_set_flip_vertically_on_load(false);
    const auto tname    = config.name.value_or(ctx.path().filename());
    const auto map_name = std::format("{}_CubeMap", tname);

    i32 width                                                  = 0, height = 0, channels = 0, size = 0;
    std::vector<std::pair<std::string, std::vector<u8>>> faces = {
        {std::string(keys::PX), {}},
        {std::string(keys::NX), {}},
        {std::string(keys::PY), {}},
        {std::string(keys::NY), {}},
        {std::string(keys::PZ), {}},
        {std::string(keys::NZ), {}},
    };

    const auto base_dir = Path{ctx.path().full_path()}.parent_path();

    try {
        const auto yaml = YAML::LoadFile(path.string());

        const auto name = fetch_optional(yaml, keys::NAME);

        for (auto& [key, data_buffer] : faces) {
            const auto& node = yaml[key];

            if (!node.IsScalar()) {
                return invalid_schema("Cube map face must be a scalar.");
            }

            const auto face_path = *FileSystem::to_physical(base_dir / node.as<std::string>());
            log::trace("Attempting to load cube map face from {}", face_path.string());

            u8* data = stbi_load(face_path.c_str(), &width, &height, &channels, 4);

            if (size == 0) {
                size = width;
            }

            if (!data || width != size || height != size) {
                if (data) {
                    stbi_image_free(data);
                }
                log::warn("Could not load image data, reason: {}", stbi_failure_reason());
            }

            data_buffer = std::vector<u8>(data, data + (size * size * 4));

            stbi_image_free(data);
        }
    } catch (const YAML::ParserException& e) {
        return invalid_format(path.string(), e.msg);
    } catch (const YAML::BadFile& e) {
        return file_not_found(path.string());
    }

    auto image = ctx.device().create_image(
        {
            .label         = map_name,
            .format        = ImageFormat::RGBA8,
            .extent        = {.width = u32(size), .height = u32(size), .depth_or_layers = 6},
            .dimension     = ImageDimension::Cube,
            .mipmap_levels = 1,
        }
    );

    ctx.device().resource_submit(
        [&](ResourceCommandRecorder& resource) {
            for (auto&& [index, pair] : std::views::enumerate(faces)) {
                auto& [key, data_buffer] = pair;
                resource.upload_to_image(
                    image.handle(),
                    std::span(data_buffer),
                    static_cast<u32>(index)
                );
            }
        }
    );

    ctx.finish(std::make_unique<Texture>(tname, std::move(image), std::move(config.sampler)));

    return {};
}
} // namespace siren
