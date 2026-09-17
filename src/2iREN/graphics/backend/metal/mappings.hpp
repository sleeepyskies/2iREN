#pragma once

#include <Metal/MTL4PipelineState.hpp>
#include <Metal/MTLPixelFormat.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLRenderPass.hpp>
#include <Metal/MTLRenderPipeline.hpp>
#include <Metal/MTLResource.hpp>
#include <Metal/MTLVertexDescriptor.hpp>
#include <utility>

#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/layout.hpp"

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

} // namespace siren::metal
