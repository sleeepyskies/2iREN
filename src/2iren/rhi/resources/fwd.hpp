#pragma once

#include "resource.hpp"


namespace siren {

class Buffer;
struct BufferDescriptor;
using BufferHandle = ResourceHandle<Buffer>;

class Framebuffer;
struct FramebufferAttachments;
struct FramebufferDescriptor;
using FramebufferHandle = ResourceHandle<Framebuffer>;

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

} // namespace siren {
