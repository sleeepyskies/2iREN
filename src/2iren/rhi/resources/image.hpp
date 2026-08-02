#pragma once

#include <optional>
#include <string>

#include "fwd.hpp"

namespace siren {
/** @brief Defines the amount of dimensions an @ref Image may have. */
enum class ImageDimension {
    /** @brief A single dimensional image. */
    D1,
    /** @brief A two-dimensional image. */
    D2,
    /** @brief A three-dimensional image, aka an array of D2 images. */
    D3,
    /** @brief A cube map image. */
    Cube
};

/** @brief Defines the size of an @ref Image. */
struct ImageExtent {
    /** @brief The desired width of the Image. */
    u32 width;
    /** @brief The desired height of the Image. */
    u32 height;
    /** @brief The desired amount of layers (for Image arrays) of depth (for 3D images) of the Image. */
    u32 depth_or_layers = 1;

    /** @brief Equality comparison operator. */
    [[nodiscard]] constexpr auto operator==(const ImageExtent& other) const -> bool = default;

    /** @brief Stringifies the @ref ImageExtent. */
    constexpr auto to_string() const noexcept -> std::string {
        return std::format("ImageExtent({}x{}x{})", width, height, depth_or_layers);
    }
};

class ImageFormat {
public:
    enum Value {
        /** @brief Invalid or uninitialized format. */
        Unknown = 0,
        /** @brief 1-channel 8-bit R. */
        R8,
        /** @brief 3-channel 8-bit linear RGB. */
        RGB8,
        /** @brief 3-channel 8-bit sRGB. */
        sRGB8,
        /** @brief 4-channel 8-bit linear RGBA. */
        RGBA8,
        /** @brief 4-channel 8-bit sRGB. */
        sRGBA8,
        /** @brief 3-channel 16-bit float HDR RGB. */
        RGB16f,
        /** @brief 2-channel 32-bit float HDR RG. */
        RG32f,
        /** @brief Depth-stencil format (24-bit depth, 8-bit stencil). */
        Depth24Stencil8,
        /** @brief Depth buffer format (32-bit depth). */
        Depth32f,
    } value;

    // ReSharper disable once CppNonExplicitConvertingConstructor
    constexpr ImageFormat(const Value v) : value(v) {}
    constexpr ImageFormat() : value(Unknown) {}

    // ReSharper disable once CppNonExplicitConversionOperator
    constexpr operator Value() const { return value; }

    /** @brief Stringifies the given ImageFormat. */
    [[nodiscard]] constexpr auto to_string() const -> std::string_view {
        switch (value) {
            case Unknown: return "Unknown";
            case R8: return "R8";
            case RGB8: return "RGB8";
            case sRGB8: return "sRGB8";
            case RGBA8: return "RGBA8";
            case sRGBA8: return "sRGBA8";
            case RGB16f: return "RGB16f";
            case RG32f: return "RG32f";
            case Depth24Stencil8: return "Depth24Stencil8";
            default: UNREACHABLE();
        }
    }
};

/**
 * @brief Describes an @ref Image for creation.
 */
struct ImageDescriptor {
    /** @brief An optional label. Mainly used for debugging. */
    std::optional<std::string> label = std::nullopt;
    /** @brief The format of the image data (num channels/bytes per channel). */
    ImageFormat format;
    /** @brief Size of the image. */
    ImageExtent extent;
    /** @brief The dimensionality of the image. */
    ImageDimension dimension;
    /** @brief How many mip map levels to generate. */
    u32 mipmap_levels;
};

/**
 * @brief A gpu resource representing image data.
 */
class Image final : public RenderResource<Image> {
    using Base = RenderResource<Image>;

public:
    Image(Device* device, ImageHandle handle);
    ~Image();
    Image(Image&& other) noexcept;
    Image& operator=(Image&& other) noexcept;

    /** @brief Returns the descriptor of this Image. */
    [[nodiscard]] auto descriptor() const noexcept -> const ImageDescriptor&;
};
} // namespace siren
