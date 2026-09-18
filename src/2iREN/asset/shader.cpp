#include "shader.hpp"

#include <string>
#include <yaml-cpp/yaml.h>

#include "2iREN/asset/asset_loader.hpp"
#include "2iREN/asset/asset_server.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/utility/filesystem.hpp"

namespace siren {
// basically just string constants to avoid typos etc
namespace keys {
constexpr std::string_view NAME   = "name";
constexpr std::string_view STAGES = "stages";
constexpr std::string_view PATH   = "path";
constexpr std::string_view SOURCE = "source";

constexpr std::string_view GLSL = "glsl";
constexpr std::string_view MSL  = "msl";

constexpr std::string_view VERTEX   = "vertex";
constexpr std::string_view FRAGMENT = "fragment";
} // namespace keys

static auto invalid_schema(const std::string_view msg) -> AssetLoadError {
    log::warn("invalid schema found: {}", msg);
    return std::unexpected(AssetErrorCode::InvalidSchema);
}

static auto file_not_found(const std::string_view path) -> AssetLoadError {
    log::warn("shader file not found at path: {}", path);
    return std::unexpected(AssetErrorCode::FileNotFound);
}

static auto invalid_format(const std::string_view path, const std::string_view msg = "")
    -> AssetLoadError {
    log::warn("invalid yaml syntax in shader file: {}. message: {}", path, msg);
    return std::unexpected(AssetErrorCode::InvalidFormat);
}

static auto fetch_optional(const YAML::Node& node, const std::string_view key)
    -> std::optional<std::string> {
    return node[key] ? std::make_optional(node[key].as<std::string>()) : std::nullopt;
}

static auto fetch_source_code(const YAML::Node& node, const Path& base_dir)
    -> std::optional<std::string> {
    const auto source = fetch_optional(node, keys::SOURCE);
    const auto path   = fetch_optional(node, keys::PATH);

    if (source) {
        return source;
    }

    if (path) {
        const auto abs = base_dir / *path;
        log::trace("attempting to load shader stage from path at {}", abs.string());
        const auto text = FileSystem::read_text(abs);
        if (!text) {
            log::warn("could not read shader source from file {}", abs.string());
            return std::nullopt;
        }
        return *text;
    }

    log::warn("shader stage has not defined a source or a path.");
    return std::nullopt;
}

auto ShaderLoader::load_glsl(
    LoadContext&&      ctx,
    const YAML::Node&  glsl,
    const std::string& name
) const -> AssetLoadError {
    if (!glsl[keys::STAGES] || !glsl[keys::STAGES].IsMap() || glsl[keys::STAGES].size() == 0) {
        return invalid_schema("'stages' key is missing or empty.");
    }

    const auto base_dir = Path{ctx.path().full_path()}.parent_path();
    std::unordered_map<ShaderStage, ShaderData> map{};

    for (const auto& stage : glsl[keys::STAGES]) {
        const auto stage_str    = stage.first.as<std::string>();
        const auto stage_node   = stage.second;
        const auto shader_stage = ShaderStage::from_string(stage_str);

        if (!stage_node.IsMap()) {
            return invalid_schema("Shader stage must be an object.");
        }

        if (!shader_stage.has_value()) {
            log::warn("unknown shader stage found ({}). continuing anyway", stage_str);
            continue;
        }

        const auto stage_name = fetch_optional(stage_node, keys::NAME);
        const auto source     = fetch_source_code(stage_node, base_dir);
        if (!source.has_value()) {
            log::warn("invalid shader stage found ({}). continuing anyway", stage_str);
            continue;
        }

        map[*shader_stage] = {
            .label  = stage_name,
            .source = *source,
        };
    }

    if (map.empty()) {
        return invalid_schema("could not parse any valid shader stages.");
    }

    auto shader = ctx.device().make_shader({.label = name, .source = map});

    ctx.finish(std::make_unique<ShaderAsset>(std::move(shader), std::move(map)));
    return {};
}

auto ShaderLoader::load_msl(
    LoadContext&&      ctx,
    const YAML::Node&  msl,
    const std::string& shader_name
) const -> AssetLoadError {
    const auto base_dir = Path{ctx.path().full_path()}.parent_path();

    const auto vertex_name   = msl[keys::VERTEX].as<std::string>();
    const auto fragment_name = msl[keys::VERTEX].as<std::string>();

    const auto source = fetch_source_code(msl, base_dir);

    if (!source.has_value()) {
        return invalid_schema("could not load msl, either source or path must be given.");
    }

    const auto map = std::unordered_map<ShaderStage, ShaderData>{
        {ShaderStage::Vertex,
         ShaderData{
             .label  = vertex_name,
             .source = *source,
             .entry  = vertex_name,
         }},
        {ShaderStage::Fragment,
         ShaderData{
             .label  = vertex_name,
             .source = *source,
             .entry  = vertex_name,
         }},
    };

    auto shader = ctx.device().make_shader({.label = shader_name, .source = map});

    ctx.finish(std::make_unique<ShaderAsset>(std::move(shader), std::move(map)));
    return {};
}

auto ShaderLoader::load(LoadContext&& ctx, const std::optional<ConfigType>) const
    -> AssetLoadError {
    const auto path = FileSystem::to_physical(ctx.path().full_path());
    if (!path.has_value()) {
        return file_not_found(FileSystem::to_physical(ctx.path().full_path())->string());
    }

    const auto backend = ctx.device().backend();

    try {
        const auto yaml = YAML::LoadFile(path->string());
        const auto name = yaml[keys::NAME].as<std::string>("unnamed");

        switch (backend) {
            case Backend::OpenGL: {
                if (!yaml[keys::GLSL]) {
                    return invalid_schema("`glsl` tag is missing, cannot load glsl shaders.");
                }
                return load_glsl(std::move(ctx), yaml[keys::GLSL], name);
            }
            case Backend::Metal: {
                if (!yaml[keys::MSL]) {
                    return invalid_schema("`msl` tag is missing, cannot load msl shaders.");
                }
                return load_msl(std::move(ctx), yaml[keys::GLSL], name);
            }
        }

    } catch (const YAML::ParserException& e) {
        return invalid_format(path->string(), e.msg);
    } catch (const YAML::BadFile& e) {
        return file_not_found(path->string());
    }
}
} // namespace siren
