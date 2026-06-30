#pragma once

#include <string>

#include "../asset_loader.hpp"
#include "2iren/rhi/resources/image.hpp"
#include "2iren/rhi/resources/sampler.hpp"


namespace siren {

/**
 * @brief Determines how to interpret texture data as a 2D TextureArray. Siren requires the
 * images are stacked on top of each other and all have the same dimensions.
 */
struct ImageArrayLayout {
    /** @brief The height of each texture in the array. */
    u32 row_height;
};

/**
 * @brief An asset holding an @ref Image and an @ref ImageSampler.
 */
struct Texture : Asset {
    /** @brief Constructs a new Texture. */
    Texture(
        const std::string& name,
        Image&& image,
        Sampler&& sampler
    ) : name(name), image(std::move(image)), sampler(std::move(sampler)) { }

    /** @brief The name of the Texture. */
    std::string name;
    /** @brief The underlying Image of the Texture. */
    Image image;
    /** @brief The underlying ImageSampler of the Texture. */
    Sampler sampler;
};

template <>
struct LoaderTraits<Texture> {
    /** @brief Loader configuration for loading texture files. */
    struct Config {
        /** @brief The name of the Texture to load. Uses filename if not present. */
        std::optional<std::string> name = std::nullopt;
        /** @brief The format of the Texture to load. Guesses if not present. */
        std::optional<ImageFormat> format = std::nullopt;
        /** @brief The sampler of the Texture to load. */
        Sampler sampler;
        /** @brief If present, determines how to interpret the array of textures. */
        std::optional<ImageArrayLayout> array_layout = std::nullopt;
        /** @brief If true, generates 1 + floor(log2(max(w, h, d))) mipmap levels. */
        bool generate_mipmap_levels = false;
    };
};

/**
 * @brief Loader for textures.
 * See @ref Texture.
 */
class TextureLoader final : public AssetLoader<Texture> {
public:
    auto load(
        LoadContext&& ctx,
        std::optional<ConfigType> config
    ) const -> AssetLoadError override;
    [[nodiscard]] auto extensions() const -> std::vector<std::string_view> override { return { "png", "jpg", "jpeg" }; }
};

} // namespace siren
