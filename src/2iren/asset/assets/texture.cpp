#include "texture.hpp"

#include <stb_image.h>
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

// todo: this only loads 2d images

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

[[nodiscard]] static auto determine_format(const TextureLoader::ConfigType& cfg, const std::string& ext)
    -> ImageFormat {
    if (cfg.format) {
        return *cfg.format;
    }

    if (ranges::contains(filetypes::SRGB, ext)) {
        log::trace("Guessing extension {} image has format LinearColor8.", ext);
        return ImageFormat::RGBA8;
    }

    if (ranges::contains(filetypes::HDR, ext)) {
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
        case ImageFormat::Depth24Stencil8:
        case ImageFormat::Unknown: return false;

        default: UNREACHABLE("Could not determine srgb from ImageFormat");
    }
}

auto TextureLoader::load(LoadContext&& ctx, std::optional<ConfigType> config) const -> AssetLoadError {
    if (!config) {
        return no_config();
    }

    const auto path = FileSystem::to_physical(ctx.path().full_path());
    if (!path) {
        return file_not_found(ctx.path());
    }
    const auto tname = config->name.value_or(ctx.path().filename());
    const auto iname = std::format("{}_Image", tname);

    const auto format = determine_format(*config, ctx.path().extension());
    // const auto is_srgb = determine_srgb(format);

    i32 width = 0, height = 0, channels = 0;
    u8* data                 = stbi_load(path->c_str(), &width, &height, &channels, 0);
    const ImageExtent extent = {
        .width           = static_cast<usize>(width),
        .height          = static_cast<usize>(height),
        .depth_or_layers = 1,
    };
    const u32 mipmap_levels = config->generate_mipmap_levels ? calc_mipmap_levels(width, height) : 1;
    if (!data) {
        log::warn("Could not load, reason: {}", stbi_failure_reason());
    }
    const usize data_size = width * height * channels;

    auto image    = ctx.device().create_image({
           .label         = iname,
           .format        = format,
           .extent        = extent,
           .dimension     = ImageDimension::D2,
           .mipmap_levels = mipmap_levels,
    });
    auto resource = ctx.device().record_resource_commands();
    resource.upload_to_image(image.handle(), std::span(data, data_size));
    ctx.device().submit(resource.finish());

    stbi_image_free(data);
    ctx.finish(std::make_unique<Texture>(tname, std::move(image), std::move(config->sampler)));

    return {};
}
} // namespace siren
