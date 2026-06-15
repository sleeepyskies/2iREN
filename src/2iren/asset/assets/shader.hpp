#pragma once

#include "2iren/asset/asset.hpp"
#include "2iren/rhi/resources/shader.hpp"
#include "../asset_loader.hpp"


namespace siren::asset {

/**
 * @brief Holds information on a single shader stage.
 */
struct ShaderStageData {
    /** @brief The optional label of the shader. */
    std::optional<std::string> label;
    /** @brief The source code of the stage. */
    std::string source;
};

/**
 * @brief An asset wrapper around a @ref Shader. Provides access to the shader,
 * as well as retains information of the shader.
 */
struct ShaderAsset : Asset {
    /** @brief The underlying GPU Shader object. */
    Shader shader;
    /** @brief The source code for each stage of the Shader. */
    std::unordered_map<ShaderStage, ShaderStageData> source;
};

template <>
struct LoaderTraits<ShaderAsset> {
    struct Config { };
};

/**
 * @brief Loader for shaders.
 * See @ref Shader
 */
class ShaderLoader final : public AssetLoader<ShaderAsset> {
public:
    auto load(
        LoadContext&& ctx,
        const ConfigType& config
    ) const -> AssetLoadError override;
    auto extensions() const -> std::vector<std::string_view> override { return { "sshg" }; }
};

} // namespace siren::asset
