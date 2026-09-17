#pragma once

#include <optional>
#include <string>

#include "2iREN/graphics/fwd.hpp"
#include "2iREN/math/color.hpp"
#include "2iREN/math/extent.hpp"
#include "2iREN/utility/byte_buffer.hpp"

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

/// @brief Represents the per pixel format of an image.
class ImageFormat {
public:
    enum Value {
        /// @brief Invalid or uninitialized format.
        Unknown = 0,
        /// @brief 1-channel 8-bit R.
        R8,
        /// @brief 1-channel 32-bit R.
        R32UI,
        /// @brief 2-channel 32-bit float HDR RG.
        RG32f,
        /// @brief 4-channel 8-bit linear RGBA.
        RGBA8,
        /// @brief 4-channel 8-bit sRGB.
        sRGBA8,
        /// @brief 4-channel 16-bit float HDR RGB.
        RGBA16f,
        /// @brief Depth-stencil format (24-bit depth, 8-bit stencil).
        Depth24Stencil8,
        /// @brief Depth buffer format (32-bit depth).
        Depth32f,
    } value;

    constexpr ImageFormat(const Value v) : value(v) { }
    constexpr ImageFormat() : value(Unknown) { }

    constexpr operator Value() const {
        return value;
    }

    /// @brief Stringifies the given ImageFormat.
    [[nodiscard]] constexpr auto to_string() const -> std::string_view {
        switch (value) {
            case R8: return "R8";
            case RGBA8: return "RGBA8";
            case sRGBA8: return "sRGBA8";
            case RGBA16f: return "RGBA16f";
            case RG32f: return "RG32f";
            case Depth24Stencil8: return "Depth24Stencil8";
            case R32UI: return "R32UI";
            case Depth32f: return "Depth32f";

            case Unknown: return "Unknown";
        }
    }

    /// @brief Returns the number of bytes per pixel for this format.
    [[nodiscard]] constexpr auto bytes_per_pixel() const -> u32 {
        switch (value) {
            case Unknown: return 0;
            case R8: return 1;
            case RGBA8: return 4;
            case sRGBA8: return 4;
            case RGBA16f: return 8;
            case RG32f: return 8;
            case Depth24Stencil8: return 4;
            case R32UI: return 4;
            case Depth32f: return 4;
        }
    }

    /// @brief Returns the number of components this format can hold.
    [[nodiscard]] constexpr auto num_components() const -> u32 {
        switch (value) {
            case Unknown: return 0;

            case R32UI:
            case Depth32f:
            case R8: return 1;

            case Depth24Stencil8:
            case RG32f: return 2;

            case RGBA8:
            case sRGBA8:
            case RGBA16f: return 4;
        }
    }
};

/// @brief Describes an @ref Image for creation.
struct ImageDescriptor {
    /// @brief An optional label.
    Label          label = std::nullopt;
    /// @brief The format of the image data (num channels/bytes per channel).
    ImageFormat    format;
    /// @brief Extent of the image.
    Extent3u       extent;
    /// @brief The dimensionality of the image.
    ImageDimension dimension;
    /// @brief How many mip map levels to generate.
    u32            mipmap_levels;
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
