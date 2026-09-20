#pragma once

#include <optional>

#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/container/flag_set.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/types.hpp"
#include "2iREN/math/extent.hpp"

namespace siren {

/// @brief Defines the amount of dimensions an @ref Image may have.
enum class ImageDimension { D1, D2, D3, Cube };

[[nodiscard]]
constexpr auto to_string(const ImageDimension dimension) -> std::string_view {
    switch (dimension) {
        case ImageDimension::D1: return "D1";
        case ImageDimension::D2: return "D2";
        case ImageDimension::D3: return "D3";
        case ImageDimension::Cube: return "Cube";
    }
}

/// @brief Defines the usage of a Buffer.
enum class ImageFlag {
    ShaderRead,
    ShaderWrite,
    RenderAttachment,
    UseAtomics,

    Max,
};

/// @brief Set of flags defining how a buffer may be used.
using ImageFlags = FlagSet<ImageFlag>;

/// @brief Describes an @ref Image for creation.
struct ImageDescriptor {
    /// @brief An optional label.
    Label label = std::nullopt;
    /// @brief The format of the image data (num channels/bytes per channel).
    ImageFormat format;
    /// @brief Extent of the image. The z-axis is used iff the image is an aray.
    Extent3u extent;
    /// @brief The dimensionality of the image.
    ImageDimension dimension = ImageDimension::D2;
    /// @brief How many mip map levels to generate.
    u32 mipmap_levels        = 1;
    /// @brief Defines if the Cpu and the Gpu mauy access to image.
    MemoryUsage memory_usage = MemoryUsage::CpuAndGpu;
    /// @brief Flags specifying how the @ref Image will be used.
    ImageFlags flags;
};

/// @brief A gpu resource representing image data.
class Image final : public RenderResource<Image> {
    using Base = RenderResource<Image>;

public:
    Image(Device* device, ImageHandle handle);
    ~Image();
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    /// @brief Returns the descriptor of this Image.
    [[nodiscard]]
    auto descriptor() const noexcept -> const ImageDescriptor&;

    /// @brief Uploads data into the image at the given mipmap level.
    auto upload(const ByteBufferView data, u32 level = 0) -> void;
};

} // namespace siren
