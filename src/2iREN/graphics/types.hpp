#pragma once

#include <string_view>

#include "2iREN/core/assert.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/utility/string.hpp"

namespace siren {

/// @brief Specifies the GPU and CPU access permissions.
enum class MemoryUsage {
    /// @brief The buffer can be accessed by the CPU and the GPU.
    CpuAndGpu,
    /// @brief The buffer can be accessed by only the GPU.
    GpuOnly,
};

/// @brief General type representing various numeric data types.
struct DataType {
    enum Value {
        Int8,
        Int16,
        Int32,
        Int64,
        UInt8,
        UInt16,
        UInt32,
        UInt64,
        Float16,
        Float32,
        Float64,
    } value;

    constexpr DataType(const Value v) : value(v) { }
    constexpr operator Value() const {
        return value;
    }

    /// @brief Returns the size of this DataType instance in bytes.
    [[nodiscard]]
    constexpr auto size_bytes() const -> usize {
        switch (value) {
            case Int8:
            case UInt8: return 1;

            case Int16:
            case UInt16:
            case Float16: return 2;

            case Int32:
            case UInt32:
            case Float32: return 4;

            case Int64:
            case UInt64:
            case Float64: return 8;
        }
    }
    /// @brief Returns the string representation of this value.
    [[nodiscard]]
    constexpr auto to_string() const -> std::string_view {
        switch (value) {
            case Int8: return "Int8";
            case Int16: return "Int16";
            case Int32: return "Int32";
            case Int64: return "Int64";
            case UInt8: return "UInt8";
            case UInt16: return "UInt16";
            case UInt32: return "UInt32";
            case UInt64: return "UInt64";
            case Float16: return "Float16";
            case Float32: return "Float32";
            case Float64: return "Float64";
        }
    }
};

/// @brief Defines the type of an index buffer/value.
class IndexType {
public:
    IndexType() : value(IndexType::UInt32) { }

    enum Value : u8 { UInt16, UInt32 } value;

    /// @brief Returns the size of this format in bytes.
    [[nodiscard]]
    constexpr auto size_bytes() const -> usize {
        switch (value) {
            case UInt16: return 2;
            case UInt32: return 4;
            default: UNREACHABLE();
        }
    }

    /// @brief Returns the string representation of this value.
    [[nodiscard]]
    constexpr auto to_string() const -> std::string_view {
        switch (value) {
            case UInt16: return "UInt16";
            case UInt32: return "UInt32";
        }
    }

    IndexType(const Value v) : value(v) { }
    constexpr operator Value() const {
        return value;
    }
};

/// @brief Represents the per pixel format of an image.
class ImageFormat {
public:
    enum Value {
        Unknown = 0,
        R8,
        R32UI,
        RG32f,
        RGBA8,
        BGRA8,
        sRGBA8,
        RGBA16f,
        Depth24Stencil8,
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
            case BGRA8: return "BGRA8";
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
            case R8: return 1;

            case Depth24Stencil8:
            case R32UI:
            case Depth32f:
            case RGBA8:
            case BGRA8:
            case sRGBA8: return 4;

            case RGBA16f:
            case RG32f: return 8;

            case Unknown: PANIC("bytes_per_pixel called on ImageFormat::Unknown.");
        }
    }

    /// @brief Returns the number of components this format can hold.
    [[nodiscard]] constexpr auto num_components() const -> u32 {
        switch (value) {
            case R32UI:
            case Depth32f:
            case R8: return 1;

            case Depth24Stencil8:
            case RG32f: return 2;

            case BGRA8:
            case RGBA8:
            case sRGBA8:
            case RGBA16f: return 4;

            case Unknown: PANIC("num_components called on ImageFormat::Unknown.");
        }
    }
};

///  @brief Enum determining how to compare two values.
enum class CompareFunction {
    /// @brief Always return true. Essentially disables the depth buffer.
    Always,
    /// @brief Always return false. Nothing is drawn.
    Never,
    /// @brief Perform new < old.
    Less,
    /// @brief Perforn new == old.
    Equal,
    /// @brief Perform new <= old.
    LessEqual,
    /// @brief Perform new > old.
    Greater,
    /// @brief Perform new >= old.
    GreaterEqual,
    /// @brief Perform new != old.
    NotEqual,
};

/// @brief Represents the various possible shader stages
struct ShaderStage {
    enum Value { Vertex, Fragment } value;

    constexpr ShaderStage(const Value v) : value(v) { }
    constexpr operator Value() const {
        return value;
    }

    [[nodiscard]]
    constexpr auto to_string() const -> std::string_view {
        switch (value) {
            case Vertex: return "Vertex";
            case Fragment: return "Fragment";
        }
    }

    [[nodiscard]]
    static auto from_string(const std::string_view str) -> std::optional<ShaderStage> {
        const std::string lower = string::tolower(str);

        if (lower == "vertex") {
            return Vertex;
        }
        if (lower == "fragment") {
            return Fragment;
        }

        return std::nullopt;
    }
};

} // namespace siren

template <>
struct std::hash<siren::ShaderStage> {
    auto operator()(const siren::ShaderStage& stage) const noexcept -> siren::usize {
        return static_cast<siren::usize>(stage.value);
    }
};
