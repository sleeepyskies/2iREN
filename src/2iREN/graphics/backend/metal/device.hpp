#pragma once

#include <Metal/MTLCommandQueue.hpp>
#include <cstddef>
#include "2iREN/graphics/backend/metal/fwd.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/graphics/fwd.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/statistics.hpp"
#include "2iREN/graphics/swapchain.hpp"

namespace siren {

struct MetalShaderDetails {
    ShaderDescriptor descriptor;
    MTL4::Compiler* compiler;
};

struct MetalSwapchainDetails {
    /// @brief The original descriptor of the object.
    SwapchainDescriptor descriptor;
    /// @brief The drawable retrieved via the MetalLayer.
    CA::MetalDrawable* drawable = nullptr;
    /// @brief The image wrapper of the next swapchain image. Is reset after
    /// each call to present.
    std::optional<ImageHandle> image = std::nullopt;
};

struct MetalDeviceState {
    RenderResourceTable<MTL::Buffer*, Buffer, BufferDescriptor> buffers = {};
    RenderResourceTable<CA::MetalLayer*, Swapchain, MetalSwapchainDetails>
        swapchains                                                         = {};
    RenderResourceTable<MTL::Library*, Shader, MetalShaderDetails> shaders = {};
    RenderResourceTable<
        MTL::RenderPipelineState*,
        GraphicsPipeline,
        GraphicsPipelineDescriptor>
        pipelines                                                     = {};
    RenderResourceTable<MTL::Texture*, Image, ImageDescriptor> images = {};
};

class MetalDevice final : public Device {
public:
    explicit MetalDevice();
    ~MetalDevice() override;

    [[nodiscard]]
    auto make_buffer(
        const BufferDescriptor& descriptor,
        std::optional<ByteBufferView> initial
    ) -> Buffer override;

    [[nodiscard]]
    auto make_image(const ImageDescriptor& descriptor) -> Image override;

    [[nodiscard]]
    auto make_sampler(const SamplerDescriptor& descriptor) -> Sampler override;

    [[nodiscard]]
    auto make_shader(const ShaderDescriptor& descriptor) -> Shader override;

    [[nodiscard]]
    auto make_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
        -> GraphicsPipeline override;

    [[nodiscard]]
    auto make_swapchain(
        const Window& window,
        const SwapchainDescriptor& descriptor
    ) -> Swapchain override;

    [[nodiscard]]
    auto make_query(const QueryDescriptor& descriptor) -> Query override;

    auto destroy_buffer(BufferHandle handle) -> void override;

    auto destroy_image(ImageHandle handle) -> void override;

    auto destroy_sampler(SamplerHandle handle) -> void override;

    auto destroy_shader(ShaderHandle handle) -> void override;

    auto destroy_graphics_pipeline(GraphicsPipelineHandle handle)
        -> void override;

    auto destroy_swapchain(SwapchainHandle handle) -> void override;

    auto destroy_query(QueryHandle handle) -> void override;

    [[nodiscard]]
    auto buffer_descriptor(BufferHandle handle) const
        -> const BufferDescriptor& override;

    [[nodiscard]]
    auto image_descriptor(ImageHandle handle) const
        -> const ImageDescriptor& override;

    [[nodiscard]]
    auto sampler_descriptor(SamplerHandle handle) const
        -> const SamplerDescriptor& override;

    [[nodiscard]]
    auto shader_descriptor(ShaderHandle handle) const
        -> const ShaderDescriptor& override;

    [[nodiscard]]
    auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& override;

    [[nodiscard]]
    auto swapchain_descriptor(SwapchainHandle handle) const
        -> const SwapchainDescriptor& override;

    [[nodiscard]]
    auto query_descriptor(QueryHandle handle) const
        -> const QueryDescriptor& override;

    auto submit(RenderPass&& pass) -> void override;

    auto upload_to_image(
        ImageHandle image,
        ByteBufferView data,
        usize layer
    ) const -> void override;

    auto upload_to_buffer(
        BufferHandle buffer,
        ByteBufferView data,
        usize offset
    ) const -> void override;

    auto clear_image(ImageHandle image, ClearValue clearvalue) const
        -> void override;

    auto blit_to_image(ImageHandle source, ImageHandle destination) const
        -> void override;

    auto read_image(ImageHandle image) const -> std::vector<u8> override;

    auto present(SwapchainHandle handle, OverlayFunction&& overlay = nullptr)
        -> void override;

    auto query_result(QueryHandle handle) const -> u64 override;

    auto query_available(QueryHandle handle) const -> bool override;

    auto begin_conditional_render(QueryHandle query) const -> void override;

    auto end_conditional_render() const -> void override;

    [[nodiscard]]
    auto limits() const -> const Limits& override;

    [[nodiscard]]
    auto statistics() const -> Statistics override;

    [[nodiscard]]
    auto acquire_next_swapchain_target(SwapchainHandle handle)
        -> ImageHandle override;

    auto wait_idle() const noexcept -> void override;

private:
    MetalDeviceState m_state               = {};
    MTL::Device* m_device                  = nullptr;
    MTL::CommandQueue* m_cmd_queue         = nullptr;
    MTL::CommandBuffer* m_cmd_buffer       = nullptr;
    NS::AutoreleasePool* m_autoreleasepool = nullptr;
    Limits m_limits                        = {};
    Statistics m_statistics                = {};
};

} // namespace siren
