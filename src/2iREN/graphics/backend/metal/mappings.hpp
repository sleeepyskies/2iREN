#pragma once

#include <Metal/MTLPixelFormat.hpp>
#include <Metal/MTLRenderCommandEncoder.hpp>
#include <Metal/MTLRenderPass.hpp>
#include <Metal/MTLRenderPipeline.hpp>
#include <Metal/MTLResource.hpp>
#include <Metal/MTLVertexDescriptor.hpp>
#include <utility>

#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/layout.hpp"
#include "2iREN/graphics/render_target.hpp"

namespace siren::metal {

constexpr auto buffer_usage_flags(const BufferUsage usage)
    -> MTL::ResourceOptions {
    switch (usage) {
        case BufferUsage::Static: return MTL::ResourceStorageModePrivate;
        case BufferUsage::Dynamic: return MTL::ResourceStorageModeShared;
    }
}

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
            case DataType::Float64:
                PANIC("64-bit vertex attributes are not supported by metal.");
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
            case DataType::Float64:
                PANIC("64-bit vertex attributes are not supported by metal.");
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
            case DataType::Float64:
                PANIC("64-bit vertex attributes are not supported by metal.");
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
            case DataType::Float64:
                PANIC("64-bit vertex attributes are not supported by metal.");
        }
    }

    PANIC("invalid vertex format.");
}

constexpr auto primtive_topology(const PrimitiveTopology topology)
    -> MTL::PrimitiveTopologyClass {
    switch (topology) {
        case PrimitiveTopology::Points: return MTL::PrimitiveTopologyClassPoint;
        case PrimitiveTopology::Lines: return MTL::PrimitiveTopologyClassLine;
        case PrimitiveTopology::Triangles:
            return MTL::PrimitiveTopologyClassTriangle;

        case PrimitiveTopology::TriangleStrip:
        case PrimitiveTopology::TriangleFan:
        case PrimitiveTopology::LineStrip:
            PANIC("metal does not support the rquested primtive topology type");
    }
}

constexpr auto primitive_type(const PrimitiveTopology topology)
    -> MTL::PrimitiveType {
    switch (topology) {
        case PrimitiveTopology::Points: return MTL::PrimitiveTypePoint;
        case PrimitiveTopology::Lines: return MTL::PrimitiveTypeLine;
        case PrimitiveTopology::Triangles: return MTL::PrimitiveTypeTriangle;

        case PrimitiveTopology::TriangleStrip:
            return MTL::PrimitiveTypeTriangleStrip;
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
        case MTL::PixelFormatDepth24Unorm_Stencil8:
            return ImageFormat::Depth24Stencil8;
        case MTL::PixelFormatDepth32Float: return ImageFormat::Depth32f;
        default:
            PANIC("unknown metal pixel format {}", std::to_underlying(format));
    }
}

constexpr auto load_action(const BeginOperation& operation) -> MTL::LoadAction {
    switch (operation) {
        case BeginOperation::Clear: return MTL::LoadActionClear;
        case BeginOperation::Preserve: return MTL::LoadActionLoad;
        case BeginOperation::Fuckit: return MTL::LoadActionDontCare;
    }
}

} // namespace siren::metal
