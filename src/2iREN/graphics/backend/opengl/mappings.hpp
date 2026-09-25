#pragma once 

#include "2iREN/graphics/backend/opengl/opengl.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/types.hpp"

namespace siren::opengl {

[[nodiscard]]
constexpr auto buffer_bitfield(const MemoryUsage usage) -> GLbitfield {
    switch (usage) {
        case MemoryUsage::Private: return 0;
        case MemoryUsage::Shared: return GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT;
    }
}

[[nodiscard]] 
constexpr auto img_filter_to_gl(const FilterMode mode) -> GLenum {
    switch (mode) {
        case FilterMode::Nearest: return GL_NEAREST;
        case FilterMode::Linear: return GL_LINEAR;
    }
    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto min_img_filter_to_gl(
    const FilterMode min,
    const FilterMode lod
) -> GLenum {
    if (min == FilterMode::Linear) {
        return lod == FilterMode::Linear ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;
    }
    return lod == FilterMode::Linear ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
}

[[nodiscard]] 
constexpr auto img_filter_to_siren(const GLenum mode) -> FilterMode {
    switch (mode) {
        case GL_NEAREST: return FilterMode::Nearest;
        case GL_LINEAR: return FilterMode::Linear;
        default: UNREACHABLE();
    }
}

[[nodiscard]] 
constexpr auto img_wrap_to_gl(const WrapMode mode) -> GLint {
    switch (mode) {
        case WrapMode::Repeat: return GL_REPEAT;
        case WrapMode::Mirror: return GL_MIRRORED_REPEAT;
        case WrapMode::ClampEdge: return GL_CLAMP_TO_EDGE;
        case WrapMode::ClampBorder: return GL_CLAMP_TO_BORDER;
    }
    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto img_wrap_to_siren(const GLenum mode) -> WrapMode {
    switch (mode) {
        case GL_REPEAT: return WrapMode::Repeat;
        case GL_MIRRORED_REPEAT: return WrapMode::Mirror;
        case GL_CLAMP_TO_EDGE: return WrapMode::ClampEdge;
        case GL_CLAMP_TO_BORDER: return WrapMode::ClampBorder;
        default: UNREACHABLE();
    }
}

[[nodiscard]] 
constexpr auto img_dim_to_gl(const ImageDimension dim) -> GLenum {
    switch (dim) {
        case ImageDimension::D1: return GL_TEXTURE_1D;
        case ImageDimension::D2: return GL_TEXTURE_2D;
        case ImageDimension::D3: return GL_TEXTURE_3D;
        case ImageDimension::Cube: PANIC();
    }
    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto img_dim_to_siren(const GLenum dim) -> ImageDimension {
    switch (dim) {
        case GL_TEXTURE_1D: return ImageDimension::D1;
        case GL_TEXTURE_2D: return ImageDimension::D2;
        case GL_TEXTURE_3D: return ImageDimension::D3;
        default: UNREACHABLE();
    }
}

[[nodiscard]] 
constexpr auto img_format_to_gl_internal(const ImageFormat format) -> GLenum {
    switch (format) {
        case ImageFormat::Unknown: return GL_INVALID_ENUM;
        case ImageFormat::R8: return GL_R8;
        case ImageFormat::R32UI: return GL_R32UI;
        case ImageFormat::RG32f: return GL_RG32F;
        case ImageFormat::RGBA8: return GL_RGBA8;
        case ImageFormat::BGRA8: return GL_BGRA8_EXT;
        case ImageFormat::sRGBA8: return GL_SRGB8_ALPHA8;
        case ImageFormat::RGBA16f: return GL_RGBA16F;
        case ImageFormat::Depth24Stencil8: return GL_DEPTH24_STENCIL8;
        case ImageFormat::Depth32f: return GL_DEPTH_COMPONENT32F;
    }
    UNREACHABLE();
}

/// @brief Converts an OpenGL internal format constant back to a 2iREN
/// ImageFormat.
[[nodiscard]] constexpr auto img_format_from_gl_internal(const GLenum internal_format)
    -> ImageFormat {
    switch (internal_format) {
        case GL_INVALID_ENUM: return ImageFormat::Unknown;
        case GL_R8: return ImageFormat::R8;
        case GL_R32UI: return ImageFormat::R32UI;
        case GL_RG32F: return ImageFormat::RG32f;
        case GL_RGBA8: return ImageFormat::RGBA8;
        case GL_BGRA8_EXT: return ImageFormat::BGRA8;
        case GL_SRGB8_ALPHA8: return ImageFormat::sRGBA8;
        case GL_RGBA16F: return ImageFormat::RGBA16f;
        case GL_DEPTH24_STENCIL8: return ImageFormat::Depth24Stencil8;
        case GL_DEPTH_COMPONENT32F: return ImageFormat::Depth32f;
        default: PANIC();
    }

}

/// @brief Maps a 2iREN format to the OpenGL pixel layout (format/type).
/// @details Defines the expected structure of CPU-side pixel data.
[[nodiscard]] constexpr auto img_format_to_gl_layout(const ImageFormat format) -> GLenum {
    switch (format) {
        case ImageFormat::R32UI:
        case ImageFormat::R8: return GL_RED;

        case ImageFormat::RG32f: return GL_RG;

        case ImageFormat::RGBA8:
        case ImageFormat::RGBA16f:
        case ImageFormat::sRGBA8: return GL_RGBA;

        case ImageFormat::BGRA8: return GL_BGRA_EXT;

        case ImageFormat::Depth24Stencil8:
        case ImageFormat::Depth32f: return GL_DEPTH_STENCIL;

        case ImageFormat::Unknown: PANIC();
          break;
        }
}

[[nodiscard]] 
constexpr auto compare_function_to_gl(const CompareFunction func) -> GLenum {
    switch (func) {
        case CompareFunction::Always: return GL_ALWAYS;
        case CompareFunction::Never: return GL_NEVER;
        case CompareFunction::Less: return GL_LESS;
        case CompareFunction::Equal: return GL_EQUAL;
        case CompareFunction::LessEqual: return GL_LEQUAL;
        case CompareFunction::Greater: return GL_GREATER;
        case CompareFunction::NotEqual: return GL_NOTEQUAL;
        case CompareFunction::GreaterEqual: return GL_GEQUAL;
    }
}

[[nodiscard]] 
constexpr auto img_compare_fn_to_siren(const GLenum func) -> CompareFunction {
    switch (func) {
        case GL_ALWAYS: return CompareFunction::Always;
        case GL_NEVER: return CompareFunction::Never;
        case GL_LESS: return CompareFunction::Less;
        case GL_EQUAL: return CompareFunction::Equal;
        case GL_LEQUAL: return CompareFunction::LessEqual;
        case GL_GREATER: return CompareFunction::Greater;
        case GL_NOTEQUAL: return CompareFunction::NotEqual;
        case GL_GEQUAL: return CompareFunction::GreaterEqual;
    }

    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto img_to_target_gl(const Extent3 extent, const ImageDimension dimension) -> GLenum {
    switch (dimension) {
        case ImageDimension::D1: return (extent.z > 1) ? GL_TEXTURE_1D_ARRAY : GL_TEXTURE_1D;
        case ImageDimension::D2: return (extent.z > 1) ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
        // There are no 3D arrays in GL.
        case ImageDimension::D3: return GL_TEXTURE_3D;
        // 6 layers = 1 cube. > 6 layers = Array of cubes.
        case ImageDimension::Cube:
            return (extent.z > 6) ? GL_TEXTURE_CUBE_MAP_ARRAY : GL_TEXTURE_CUBE_MAP;
    }
    UNREACHABLE();
}

/// @brief Converts a siren @ref ShaderStage to its native GLenum version.
/// @param shader_stage The @ref ShaderStage to convert.
/// @return A converted GLenum.
[[nodiscard]] 
constexpr auto shader_stage_to_gl(const ShaderStage shader_stage) -> GLenum {
    switch (shader_stage) {
        case ShaderStage::Vertex: return GL_VERTEX_SHADER;
        case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
    }
}

[[nodiscard]] 
constexpr auto blend_function_to_gl(const BlendFunction blend_function) -> GLenum {
    switch (blend_function) {
        case BlendFunction::Add: return GL_FUNC_ADD;
        case BlendFunction::Subtract: return GL_FUNC_SUBTRACT;
        case BlendFunction::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
        case BlendFunction::Min: return GL_MIN;
        case BlendFunction::Max: return GL_MAX;
    }
    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto blend_factor_to_gl(const BlendFactor factor) -> GLenum {
    switch (factor) {
        case BlendFactor::Zero: return GL_ZERO;
        case BlendFactor::One: return GL_ONE;
        case BlendFactor::SourceAlpha: return GL_SRC_ALPHA;
        case BlendFactor::OneMinusSourceAlpha: return GL_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DestinationAlpha: return GL_DST_ALPHA;
        case BlendFactor::OneMinusDestinationAlpha: return GL_ONE_MINUS_DST_ALPHA;
    }
    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto topology_to_gl(const PrimitiveTopology topology) -> GLenum {
    switch (topology) {
        case PrimitiveTopology::Points: return GL_POINTS;
        case PrimitiveTopology::Lines: return GL_LINES;
        case PrimitiveTopology::LineStrip: return GL_LINE_STRIP;
        case PrimitiveTopology::Triangles: return GL_TRIANGLES;
        case PrimitiveTopology::TriangleStrip: return GL_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan: return GL_TRIANGLE_FAN;
    }
    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto index_format_to_gl(const IndexType type) -> GLenum {
    switch (type) {
        case IndexType::UInt16: GL_UNSIGNED_SHORT;
        case IndexType::UInt32: GL_UNSIGNED_INT;
    }

    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto siren_datatype_to_gl(const DataType type) -> GLenum {
    switch (type) {
        case DataType::Int8: return GL_BYTE;
        case DataType::Int16: return GL_SHORT;
        case DataType::Int32: return GL_INT;
        case DataType::Int64: return GL_INT64_ARB;

        case DataType::UInt8: return GL_UNSIGNED_BYTE;
        case DataType::UInt16: return GL_UNSIGNED_SHORT;
        case DataType::UInt32: return GL_UNSIGNED_INT;
        case DataType::UInt64: return GL_UNSIGNED_INT64_ARB;

        case DataType::Float16: return GL_HALF_FLOAT;
        case DataType::Float32: return GL_FLOAT;
        case DataType::Float64: return GL_DOUBLE;
    }
    UNREACHABLE();
}

[[nodiscard]] 
constexpr auto query_kind_to_gl(const QueryKind kind) -> GLenum {
    switch (kind) {
        case QueryKind::SamplesPassed: return GL_SAMPLES_PASSED;
        case QueryKind::AnySamplesPassed: return GL_ANY_SAMPLES_PASSED;
        case QueryKind::TimeElapsed: return GL_TIME_ELAPSED;
        default: UNREACHABLE();
    }
}

}
