#pragma once

#include <string>
#include <optional>
#include <unordered_map>

#include "fwd.hpp"


namespace siren {
/** @brief Represents the various possible shader stages. */
struct ShaderStage {
    enum Value {
        /** @brief A Vertex Shader. */
        Vertex,
        /** @brief A Fragment Shader. */
        Fragment,
        /** @brief A Geometry Shader (optional). */
        Geometry,
        /** @brief A Compute Shader. */
        Compute,
        /** @brief A Task Shader (optional). */
        Task,
        /** @brief A Mesh Shader (replaces Vertex + Fragment stages). */
        Mesh,
    } value;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr ShaderStage(const Value v) : value(v) { }
    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator Value() const { return value; }

    /** @brief Returns the string representation of this value. */
    constexpr auto to_string() const -> std::string_view {
        switch (value) {
            case Vertex: return "Vertex";
            case Fragment: return "Fragment";
            case Geometry: return "Geometry";
            case Compute: return "Compute";
            case Task: return "Task";
            case Mesh: return "Mesh";
            default: return "Unknown";
        }
    }
};

/**
 * @brief Holds information on a single shader stage.
 */
struct ShaderData {
    /** @brief The optional label of the shader. */
    std::optional<std::string> label;
    /** @brief The source code of the stage. */
    std::string source;
};
}


template <>
struct std::hash<siren::ShaderStage> {
    auto operator()(const siren::ShaderStage& stage) const noexcept -> siren::usize {
        return static_cast<siren::usize>(stage.value);
    }
};


namespace siren {

/**
 * @brief Describes a @ref Shader to be created.
 */
struct ShaderDescriptor {
    /** @brief The optional label of the shader. */
    std::optional<std::string> label;
    /** @brief The shader code for each stage of the Shader. */
    std::unordered_map<ShaderStage, ShaderData> source;
};

/**
 * @brief Represents a shader compiled on the GPU. Provides an interface to interact
 * with the GPU shader object. However, does not retain any information post compile.
 * To recover information after compiling, see @ref ShaderAsset.
 */
class Shader : public RenderResource<Shader> {
    using Base = RenderResource<Shader>;

public:
    Shader(Device* device, ShaderHandle handle);
    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /** @brief Returns the @ref ShaderDescriptor associated with this shader. */
    [[nodiscard]] auto descriptor() const noexcept -> const ShaderDescriptor&;
};

} // namespace siren
