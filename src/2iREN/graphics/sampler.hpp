#pragma once

#include <optional>

#include "2iREN/graphics/fwd.hpp"

namespace siren {

/// @brief Tells the gpu how to filter the image.
enum class FilterMode {
    /// @brief Takes the value of the nearest neighbor.
    Nearest,
    /// @brief Bilinear filtering. Interpolates color from neighboring pixels.
    Linear,
};

/// @brief Tells the gpu how to handle coordinated outside the range [0, 1].
enum class WrapMode {
    /// @brief Repeats the image.
    Repeat,
    /// @brief Mirrors the image.
    Mirror,
    /// @brief Clamps the image color to the image border color.
    ClampEdge,
    /// @brief Coordinates outside the range are given a user defined color.
    ClampBorder,
};

/// @brief Describes the ImageSampler for creation.
struct SamplerDescriptor {
    /// @brief An optional label.
    Label label           = std::nullopt;
    /// @brief Tells the gpu how to filter when the source image is smaller.
    FilterMode min_filter = FilterMode::Nearest;
    /// @brief Tells the gpu how to filter when the source image is larger.
    FilterMode mag_filter = FilterMode::Nearest;
    /// @brief Tells the gpu how to wrap along the horizontal axis.
    WrapMode s_wrap       = WrapMode::Repeat;
    /// @brief Tells the gpu how to wrap along the vertical axis.
    WrapMode t_wrap       = WrapMode::Repeat;
    /// @brief Tells the gpu how to wrap along the depth axis.
    WrapMode r_wrap       = WrapMode::Repeat;
};

/// @brief A gpu resource defining how to read from an Image.
class Sampler : public RenderResource<Sampler> {
    using Base = RenderResource<Sampler>;

public:
    explicit Sampler(Device* device, SamplerHandle handle);
    ~Sampler();

    Sampler(Sampler&& other) noexcept;
    Sampler& operator=(Sampler&& other) noexcept;

    /// @brief Returns the Sampler's descriptor used to create it.
    [[nodiscard]] auto descriptor() const noexcept -> const SamplerDescriptor&;
};

} // namespace siren
