#pragma once

#include "2iren/asset/asset.hpp"
#include "2iren/rhi/resources/shader.hpp"
#include "../asset_loader.hpp"


namespace siren {

/**
 * @brief An asset wrapper around a @ref Shader. Provides access to the shader,
 * as well as retains information of the shader.
 */
struct ShaderAsset : Asset {
    ShaderAsset(
        Shader&& shader,
        const std::unordered_map<ShaderStage, ShaderData>& data
    ) : shader(std::move(shader)), source(data) { }

    /** @brief The underlying GPU Shader object. */
    Shader shader;
    /** @brief The source code for each stage of the Shader. */
    std::unordered_map<ShaderStage, ShaderData> source;
};

template <>
struct LoaderTraits<ShaderAsset> {
    struct Config { };
};

/**
 * @brief Loader for shaders.
 * See @ref Shader
 *
 */
class ShaderLoader final : public AssetLoader<ShaderAsset> {
public:
    auto load(
        LoadContext&& ctx,
        std::optional<ConfigType> config
    ) const -> AssetLoadError override;
    auto extensions() const -> std::vector<std::string_view> override { return { "sshg" }; }
};

} // namespace siren
