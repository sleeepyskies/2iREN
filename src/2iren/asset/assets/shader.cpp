#include "shader.hpp"

#include <yaml-cpp/yaml.h>

#include "2iren/asset/asset_server.hpp"
#include "2iren/rhi/device.hpp"
#include "2iren/util/filesystem.hpp"


namespace siren {

// basically just string constants to avoid typos etc
namespace keys {
constexpr std::string_view NAME   = "name";
constexpr std::string_view STAGES = "stages";
constexpr std::string_view PATH   = "path";
constexpr std::string_view SOURCE = "source";
} // namespace keys

static auto invalid_schema(const std::string_view msg) -> AssetLoadError {
    log::warn("Invalid Schema found: {}", msg);
    return std::unexpected(AssetErrorCode::InvalidSchema);
}

static auto file_not_found(const std::string_view path) -> AssetLoadError {
    log::warn("Shader file not found at path: {}", path);
    return std::unexpected(AssetErrorCode::FileNotFound);
}

static auto invalid_format(const std::string_view path, const std::string_view msg = "") -> AssetLoadError {
    log::warn("Invalid YAML syntax in shader file: {}. Message: {}", path, msg);
    return std::unexpected(AssetErrorCode::InvalidFormat);
}

static auto fetch_optional(const YAML::Node& node, const std::string_view key) -> std::optional<std::string> {
    return node[key] ? std::make_optional(node[key].as<std::string>()) : std::nullopt;
}

static auto fetch_source_code(const YAML::Node& node, const Path& base_dir) -> std::optional<std::string> {
    const auto source = fetch_optional(node, keys::SOURCE);
    const auto path   = fetch_optional(node, keys::PATH);

    if (source) { return source; }

    if (path) {
        const auto abs = base_dir / *path;
        log::trace("Attempting to load shader stage from path at {}", abs.string());
        const auto text = FileSystem::read_text(abs);
        if (!text) {
            log::warn("Could not read shader source from file {}", abs.string());
            return std::nullopt;
        }
        return *text;
    }

    log::warn("Shader stage has not defined a source or a path.");
    return std::nullopt;
}

auto ShaderLoader::load(
    LoadContext&& ctx,
    const std::optional<ConfigType>
) const -> AssetLoadError {
    const auto path = FileSystem::to_physical(ctx.path().full_path());
    if (!path.has_value()) { return file_not_found(ctx.path().full_path()); }

    try {
        const auto yaml = YAML::LoadFile(path->string());
        if (!yaml[keys::STAGES] || !yaml[keys::STAGES].IsMap() || yaml[keys::STAGES].size() == 0) {
            return invalid_schema("'stages' key is missing or empty.");
        }

        const auto shader_name = fetch_optional(yaml, keys::NAME);
        const auto base_dir    = Path{ ctx.path().full_path() }.parent_path();
        std::unordered_map<ShaderStage, ShaderData> map{ };

        for (const auto& stage : yaml[keys::STAGES]) {
            const auto stage_str    = stage.first.as<std::string>();
            const auto stage_node   = stage.second;
            const auto shader_stage = ShaderStage::from_string(stage_str);

            if (!stage_node.IsMap()) {
                return invalid_schema("Shader stage must be an object.");
            }

            if (!shader_stage.has_value()) {
                log::warn("Unknown shader stage found ({}). Continuing anyway", stage_str);
                continue;
            }

            const auto stage_name = fetch_optional(stage_node, keys::NAME);
            const auto source     = fetch_source_code(stage_node, base_dir);
            if (!source.has_value()) {
                log::warn("Invalid shader stage found ({}). Continuing anyway", stage_str);
                continue;
            }

            map[*shader_stage] = {
                .label = shader_name,
                .source = *source,
            };
        }

        if (map.empty()) {
            return invalid_schema("Could not parse any valid shader stages.");
        }

        auto shader = ctx.device().create_shader(
            {
                .label = shader_name,
                .source = map
            }
        );

        ctx.finish(std::make_unique<ShaderAsset>(std::move(shader), std::move(map)));
        return { };
    } catch (const YAML::ParserException& e) {
        return invalid_format(path->string(), e.msg);
    } catch (const YAML::BadFile& e) {
        return file_not_found(path->string());
    }
}

} // namespace siren
