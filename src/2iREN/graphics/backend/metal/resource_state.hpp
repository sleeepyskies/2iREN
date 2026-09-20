#pragma once

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"

namespace siren::metal {

struct ShaderDetails {
    ShaderDescriptor descriptor;
    MTL4::Compiler* compiler;
};

struct SwapchainDetails {
    /// @brief The original descriptor of the object.
    SwapchainInfo descriptor;
    /// @brief The drawable retrieved via the MetalLayer.
    NS::SharedPtr<CA::MetalDrawable> drawable = nullptr;
    /// @brief The image wrapper of the next swapchain image. Is reset after
    /// each call to present.
    std::optional<ImageHandle> image          = std::nullopt;
};

/// @brief Encapsulates the state of all alive Metal resources.
struct ResourceState {
    template <typename M, typename S, typename D>
    using Table = RenderResourceTable<M, S, D>;

    Table<NS::SharedPtr<MTL::Buffer>, Buffer, BufferDescriptor> buffers    = {};
    mutable Table<CA::MetalLayer*, Swapchain, SwapchainDetails> swapchains = {};
    Table<NS::SharedPtr<MTL::Library>, Shader, ShaderDetails> shaders      = {};
    Table<NS::SharedPtr<MTL::RenderPipelineState>, GraphicsPipeline, GraphicsPipelineDescriptor>
        pipelines                                                                = {};
    Table<NS::SharedPtr<MTL::Texture>, Image, ImageDescriptor> images            = {};
    Table<NS::SharedPtr<MTL::SamplerState>, Sampler, SamplerDescriptor> samplers = {};
};

} // namespace siren::metal
