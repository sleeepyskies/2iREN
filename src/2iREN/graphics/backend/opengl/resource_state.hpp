#pragma once 

#include <GLFW/glfw3.h>

#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/resource.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/graphics/backend/opengl/opengl.hpp"

namespace siren::opengl {

struct SwapchainDetails {
    SwapchainDescriptor descriptor;
    GLFWwindow* native_handle;
    GLuint framebuffer;
};

struct QueryDetails {
    QueryDescriptor descriptor;
};

/// @struct RenderResourceState
/// @brief Encapsulates all @ref RenderResource state for the OpenGL backend.
struct ResourceState {
    template <typename ID, typename R, typename D>
    using Table = RenderResourceTable<ID, R, D>;

    Table<GLuint, Buffer, BufferDescriptor> buffers;
    Table<GLuint, Image, ImageDescriptor> images;
    Table<GLuint, Sampler, SamplerDescriptor> samplers;
    Table<GLuint, Shader, ShaderDescriptor> shaders;
    /// @note The GLuint stored here is not of the Pipeline, but rather the
    /// vertex array object.
    Table<GLuint, GraphicsPipeline, GraphicsPipelineDescriptor> pipelines;
    /// @note GL doesn't have an exposed concept of a swapchain, so we store a
    /// meaningless void* here.
    Table<void*, Swapchain, SwapchainDetails> swapchains;
    /// @brief Manages fetching cached OpenGL framebuffers based on images.
    /// TODO: uncomment this!!
    // mutable FramebufferCache framebuffer_cache{image_table};
};

}
