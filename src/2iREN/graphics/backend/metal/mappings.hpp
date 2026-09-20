#pragma once

#include <Metal/MTL4PipelineState.hpp>
#include <Metal/MTLDepthStencil.hpp>
#include <Metal/MTLPixelFormat.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLRenderPass.hpp>
#include <Metal/MTLRenderPipeline.hpp>
#include <Metal/MTLResource.hpp>
#include <Metal/MTLSampler.hpp>
#include <Metal/MTLTexture.hpp>
#include <Metal/MTLTypes.hpp>
#include <Metal/MTLVertexDescriptor.hpp>
#include <utility>

#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/layout.hpp"
#include "2iREN/graphics/sampler.hpp"

namespace siren::metal {

/// @brief Maps a @ref Component to its corresponding MTL::VertexFormat
/// enumeration value.
constexpr auto vertex_format(const Component& component) -> MTL::VertexFormat {
    if (component.size == 1) {
        switch (component.type) {
            case DataType::Int8: return MTL::VertexFormatChar;
            case DataType::Int16: return MTL::VertexFormatShort;
            case DataType::Int32: return MTL::VertexFormatInt;

            case DataType::UInt8: return MTL::VertexFormatUChar;
            case DataType::UInt16: return MTL::VertexFormatUShort;
            case DataType::UInt32: return MTL::VertexFormatUInt;

            case DataType::Float16: return MTL::VertexFormatHalf;
            case DataType::Float32: return MTL::VertexFormatFloat;

            case DataType::Int64:
            case DataType::UInt64:
            case DataType::Float64: PANIC("64-bit vertex attributes are not supported by metal.");
        }
    } else if (component.size == 2) {
        switch (component.type) {
            case DataType::Int8: return MTL::VertexFormatChar2;
            case DataType::Int16: return MTL::VertexFormatShort2;
            case DataType::Int32: return MTL::VertexFormatInt2;

            case DataType::UInt8: return MTL::VertexFormatUChar2;
            case DataType::UInt16: return MTL::VertexFormatUShort2;
            case DataType::UInt32: return MTL::VertexFormatUInt2;

            case DataType::Float16: return MTL::VertexFormatHalf2;
            case DataType::Float32: return MTL::VertexFormatFloat2;

            case DataType::Int64:
            case DataType::UInt64:
            case DataType::Float64: PANIC("64-bit vertex attributes are not supported by metal.");
        }
    } else if (component.size == 3) {
        switch (component.type) {
            case DataType::Int8: return MTL::VertexFormatChar3;
            case DataType::Int16: return MTL::VertexFormatShort3;
            case DataType::Int32: return MTL::VertexFormatInt3;

            case DataType::UInt8: return MTL::VertexFormatUChar3;
            case DataType::UInt16: return MTL::VertexFormatUShort3;
            case DataType::UInt32: return MTL::VertexFormatUInt3;

            case DataType::Float16: return MTL::VertexFormatHalf3;
            case DataType::Float32: return MTL::VertexFormatFloat3;

            case DataType::Int64:
            case DataType::UInt64:
            case DataType::Float64: PANIC("64-bit vertex attributes are not supported by metal.");
        }
    } else if (component.size == 4) {
        switch (component.type) {
            case DataType::Int8: return MTL::VertexFormatChar4;
            case DataType::Int16: return MTL::VertexFormatShort4;
            case DataType::Int32: return MTL::VertexFormatInt4;

            case DataType::UInt8: return MTL::VertexFormatUChar4;
            case DataType::UInt16: return MTL::VertexFormatUShort4;
            case DataType::UInt32: return MTL::VertexFormatUInt4;

            case DataType::Float16: return MTL::VertexFormatHalf4;
            case DataType::Float32: return MTL::VertexFormatFloat4;

            case DataType::Int64:
            case DataType::UInt64:
            case DataType::Float64: PANIC("64-bit vertex attributes are not supported by metal.");
        }
    }

    PANIC("invalid vertex format.");
}

constexpr auto primtive_topology(const PrimitiveTopology topology) -> MTL::PrimitiveTopologyClass {
    switch (topology) {
        case PrimitiveTopology::Points: return MTL::PrimitiveTopologyClassPoint;
        case PrimitiveTopology::Lines: return MTL::PrimitiveTopologyClassLine;
        case PrimitiveTopology::Triangles: return MTL::PrimitiveTopologyClassTriangle;

        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleFan:
        case PrimitiveTopology::LineStrip:
            PANIC("metal does not support the rquested primtive topology type");
    }
}

constexpr auto primitive_type(const PrimitiveTopology topology) -> MTL::PrimitiveType {
    switch (topology) {
        case PrimitiveTopology::Points: return MTL::PrimitiveTypePoint;
        case PrimitiveTopology::Lines: return MTL::PrimitiveTypeLine;
        case PrimitiveTopology::Triangles: return MTL::PrimitiveTypeTriangle;

        case PrimitiveTopology::TriangleStrip: return MTL::PrimitiveTypeTriangleStrip;
        case PrimitiveTopology::LineStrip: return MTL::PrimitiveTypeLineStrip;
        case PrimitiveTopology::TriangleFan:
            PANIC("metal does not support the rquested primtive type");
    }
}

constexpr auto index_type(const IndexFormat format) -> MTL::IndexType {
    switch (format) {
        case IndexFormat::UInt16: return MTL::IndexType::IndexTypeUInt16;
        case IndexFormat::UInt32: return MTL::IndexType::IndexTypeUInt32; break;
    }
}

constexpr auto image_format(const MTL::PixelFormat format) -> ImageFormat {
    switch (format) {
        case MTL::PixelFormatR8Uint: return ImageFormat::R8;
        case MTL::PixelFormatR32Uint: return ImageFormat::R32UI;
        case MTL::PixelFormatRG32Float: return ImageFormat::RG32f;
        case MTL::PixelFormatBGRA8Unorm: // TODO: is this fine?
        case MTL::PixelFormatRGBA8Uint: return ImageFormat::RGBA8;
        case MTL::PixelFormatRGBA8Unorm_sRGB: return ImageFormat::sRGBA8;
        case MTL::PixelFormatRGBA16Float: return ImageFormat::RGBA16f;
        case MTL::PixelFormatDepth24Unorm_Stencil8: return ImageFormat::Depth24Stencil8;
        case MTL::PixelFormatDepth32Float: return ImageFormat::Depth32f;
        default: PANIC("unknown metal pixel format {}", std::to_underlying(format));
    }
}

constexpr auto load_action(const BeginOperation& operation) -> MTL::LoadAction {
    switch (operation) {
        case BeginOperation::Clear: return MTL::LoadActionClear;
        case BeginOperation::Preserve: return MTL::LoadActionLoad;
        case BeginOperation::Fuckit: return MTL::LoadActionDontCare;
    }
}

constexpr auto store_action(const EndOperation& operation) -> MTL::StoreAction {
    switch (operation) {
        case EndOperation::Store: return MTL::StoreActionStore;
        case EndOperation::Fuckit: return MTL::StoreActionDontCare;
    }
}

[[nodiscard]]
constexpr auto blend_operation(const BlendFunction function) -> MTL::BlendOperation {
    switch (function) {
        case BlendFunction::Add: return MTL::BlendOperationAdd;
        case BlendFunction::Subtract: return MTL::BlendOperationSubtract;
        case BlendFunction::ReverseSubtract: return MTL::BlendOperationReverseSubtract;
        case BlendFunction::Min: return MTL::BlendOperationMin;
        case BlendFunction::Max: return MTL::BlendOperationMax;
    }
}

[[nodiscard]]
constexpr auto blending_state(const AlphaMode mode) -> MTL4::BlendState {
    switch (mode) {
        case AlphaMode::Opaque: return MTL4::BlendStateDisabled;
        case AlphaMode::Blend: return MTL4::BlendStateEnabled;
    }
}

[[nodiscard]]
constexpr auto blend_factor(const BlendFactor factor) -> MTL::BlendFactor {
    switch (factor) {
        case BlendFactor::Zero: return MTL::BlendFactorZero;
        case BlendFactor::One: return MTL::BlendFactorOne;
        case BlendFactor::SourceAlpha: return MTL::BlendFactorSourceAlpha;
        case BlendFactor::DestinationAlpha: return MTL::BlendFactorDestinationAlpha;
        case BlendFactor::OneMinusSourceAlpha: return MTL::BlendFactorOneMinusSourceAlpha;
        case BlendFactor::OneMinusDestinationAlpha: return MTL::BlendFactorDestinationAlpha;
    }
}

[[nodiscard]]
constexpr auto pixel_format(const ImageFormat format) -> MTL::PixelFormat {
    switch (format) {
        case ImageFormat::Unknown: return MTL::PixelFormatInvalid;
        case ImageFormat::R8: return MTL::PixelFormatR8Uint;
        case ImageFormat::R32UI: return MTL::PixelFormatR32Uint;
        case ImageFormat::RG32f: return MTL::PixelFormatRG32Float;
        case ImageFormat::RGBA8: return MTL::PixelFormatRGBA8Unorm;
        case ImageFormat::sRGBA8: return MTL::PixelFormatRGBA8Unorm_sRGB;
        case ImageFormat::RGBA16f: return MTL::PixelFormatRGBA16Float;
        case ImageFormat::Depth24Stencil8: return MTL::PixelFormatDepth24Unorm_Stencil8;
        case ImageFormat::Depth32f: return MTL::PixelFormatDepth32Float;
    }
}

[[nodiscard]]
constexpr auto cull_mode(const CullMode mode) -> MTL::CullMode {
    switch (mode) {
        case CullMode::None: return MTL::CullModeNone;
        case CullMode::Front: return MTL::CullModeFront;
        case CullMode::Back: return MTL::CullModeBack;
    }
}

[[nodiscard]]
constexpr auto compare_function(const DepthFunction function) -> MTL::CompareFunction {
    switch (function) {
        case DepthFunction::Always: return MTL::CompareFunctionAlways;
        case DepthFunction::Never: return MTL::CompareFunctionNever;
        case DepthFunction::Less: return MTL::CompareFunctionLess;
        case DepthFunction::Equal: return MTL::CompareFunctionEqual;
        case DepthFunction::LessEqual: return MTL::CompareFunctionLessEqual;
        case DepthFunction::Greater: return MTL::CompareFunctionGreater;
        case DepthFunction::GreaterEqual: return MTL::CompareFunctionGreaterEqual;
        case DepthFunction::NotEqual: return MTL::CompareFunctionNotEqual;
    }
}

[[nodiscard]]
constexpr auto texture_type(const ImageDimension dimension) -> MTL::TextureType {
    switch (dimension) {
        case ImageDimension::D1: return MTL::TextureType1D;
        case ImageDimension::D2: return MTL::TextureType2D;
        case ImageDimension::D3: return MTL::TextureType3D;
        case ImageDimension::Cube: return MTL::TextureTypeCube;
    }
}

[[nodiscard]]
constexpr auto resource_options(const BufferMemoryUsage memory_usage) -> MTL::ResourceOptions {
    switch (memory_usage) {
        case BufferMemoryUsage::CpuAndGpu: return MTL::ResourceStorageModeShared;
        case BufferMemoryUsage::GpuOnly: return MTL::ResourceStorageModePrivate;
    }
}

[[nodiscard]]
constexpr auto resource_options(const ImageFlags flags) -> MTL::ResourceOptions {
    MTL::ResourceOptions opts = 0;

    ASSERT(
        !flags.all(ImageFlag::Shared, ImageFlag::Private),
        "cannot have private and shared image usage."
    );
    if (flags.test(ImageFlag::Shared)) {
        opts |= MTL::ResourceStorageModeShared;
    }

    if (flags.test(ImageFlag::Private)) {
        opts |= MTL::ResourceStorageModePrivate;
    }

    return opts;
}

[[nodiscard]]
constexpr auto texture_usage(const ImageFlags flags) -> MTL::TextureUsage {
    MTL::TextureUsage usage = 0;

    if (flags.test(ImageFlag::ShaderRead)) {
        usage |= MTL::TextureUsageShaderRead;
    }

    if (flags.test(ImageFlag::ShaderWrite)) {
        usage |= MTL::TextureUsageShaderWrite;
    }

    if (flags.test(ImageFlag::RenderAttachment)) {
        usage |= MTL::TextureUsageRenderTarget;
    }

    if (flags.test(ImageFlag::UseAtomics)) {
        usage |= MTL::TextureUsageShaderAtomic;
    }

    return usage;
}

[[nodiscard]]
constexpr auto region(const Extent3u extent) -> MTL::Region {
    MTL::Region region = {};

    region.origin     = MTL::Origin{0, 0, 0};
    region.size.width = extent.x;

    if (extent.y > 0) {
        region.size.height = extent.y;
    }

    if (extent.z > 0) {
        region.size.depth = extent.z;
    }

    return region;
}

[[nodiscard]]
constexpr auto address_mode(const WrapMode mode) -> MTL::SamplerAddressMode {
    switch (mode) {
        case WrapMode::Repeat: return MTL::SamplerAddressModeRepeat;
        case WrapMode::Mirror: return MTL::SamplerAddressModeMirrorRepeat;
        case WrapMode::ClampEdge: return MTL::SamplerAddressModeClampToEdge;
        case WrapMode::ClampBorder: return MTL::SamplerAddressModeClampToBorderColor;
    }
}

[[nodiscard]]
constexpr auto minmag_filter(const FilterMode mode) -> MTL::SamplerMinMagFilter {
    switch (mode) {
        case FilterMode::Nearest: return MTL::SamplerMinMagFilterNearest;
        case FilterMode::Linear: return MTL::SamplerMinMagFilterLinear;
    }
}

} // namespace siren::metal
