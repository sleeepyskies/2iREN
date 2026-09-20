#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/types.hpp"

namespace siren {

/// @brief Holds information on a single shader stage.
struct ShaderData {
    /// @brief The optional label of the shader.
    Label label;
    /// @brief The source code of the stage.
    std::string source;
    /// @brief The name of the entry function.
    std::string entry = "main";
};

/// @brief Describes a @ref Shader to be created.
struct ShaderDescriptor {
    /// @brief An optional label.
    Label label = std::nullopt;
    /// @brief The shader code for each stage of the Shader.
    std::unordered_map<ShaderStage, ShaderData> source;
};

/// @brief Represents a shader compiled on the GPU. Provides an interface to
/// interact with the GPU shader object. However, does not retain any information
/// post compile. To recover information after compiling, see @ref ShaderAsset.
class Shader : public RenderResource<Shader> {
    using Base = RenderResource<Shader>;

public:
    Shader(Device* device, ShaderHandle handle);
    ~Shader();

    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    /// @brief Returns the @ref ShaderDescriptor associated with this shader.
    [[nodiscard]]
    auto descriptor() const noexcept -> const ShaderDescriptor&;
};

} // namespace siren
