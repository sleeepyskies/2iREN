#pragma once

#include "resource.hpp"


namespace siren {

class Buffer;
struct BufferDescriptor;
using BufferHandle = ResourceHandle<Buffer>;

class GraphicsPipeline;
struct GraphicsPipelineDescriptor;
using GraphicsPipelineHandle = ResourceHandle<GraphicsPipeline>;

class Image;
struct ImageDescriptor;
using ImageHandle = ResourceHandle<Image>;

class Sampler;
struct SamplerDescriptor;
using SamplerHandle = ResourceHandle<Sampler>;

class Shader;
struct ShaderDescriptor;
using ShaderHandle = ResourceHandle<Shader>;

class Swapchain;
struct SwapchainDescriptor;
using SwapchainHandle = ResourceHandle<Swapchain>;

class Query;
struct QueryDescriptor;
using QueryHandle = ResourceHandle<Query>;

} // namespace siren {
