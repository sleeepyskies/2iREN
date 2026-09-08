#pragma once

#include "2iREN/graphics/device.hpp"

namespace siren {

class MetalDevice final : public Device {
public:
    explicit MetalDevice();
    ~MetalDevice() override;

    auto wait_idle() const noexcept -> void override;

    [[nodiscard]] auto create_buffer(const BufferDescriptor& descriptor) -> Buffer override;
    auto destroy_buffer(BufferHandle handle) -> void override;

    [[nodiscard]] auto create_image(const ImageDescriptor& descriptor) -> Image override;
    auto destroy_image(ImageHandle handle) -> void override;

    [[nodiscard]] auto create_sampler(const SamplerDescriptor& descriptor) -> Sampler override;
    auto destroy_sampler(SamplerHandle handle) -> void override;

    [[nodiscard]] auto create_shader(const ShaderDescriptor& descriptor) -> Shader override;
    auto destroy_shader(ShaderHandle handle) -> void override;

    [[nodiscard]] auto create_swapchain(const SwapchainDescriptor& descriptor)
        -> Swapchain override;
    auto destroy_swapchain(SwapchainHandle handle) -> void override;

    [[nodiscard]] auto create_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
        -> GraphicsPipeline override;
    auto destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void override;

    [[nodiscard]] auto create_query(const QueryDescriptor& descriptor) -> Query override;
    auto destroy_query(QueryHandle handle) -> void override;

    [[nodiscard]] auto record_resource_commands() const -> ResourceCommandRecorder override;
    [[nodiscard]] auto record_render_commands() const -> RenderCommandRecorder override;
    auto submit(ResourceCommandBuffer&& command_buffer) const -> void override;
    auto submit(RenderCommandBuffer&& command_buffer) const -> void override;

    [[nodiscard]] auto buffer_descriptor(BufferHandle handle) const
        -> const BufferDescriptor& override;
    [[nodiscard]] auto image_descriptor(ImageHandle handle) const
        -> const ImageDescriptor& override;
    [[nodiscard]] auto sampler_descriptor(SamplerHandle handle) const
        -> const SamplerDescriptor& override;
    [[nodiscard]] auto shader_descriptor(ShaderHandle handle) const
        -> const ShaderDescriptor& override;
    [[nodiscard]] auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& override;
    [[nodiscard]] auto swapchain_descriptor(SwapchainHandle handle) const
        -> const SwapchainDescriptor& override;
    [[nodiscard]] auto query_descriptor(QueryHandle handle) const
        -> const QueryDescriptor& override;

    auto query_result(QueryHandle handle) const -> u64 override;

    auto begin_conditional_render(const QueryHandle query) const -> void override;
    auto end_conditional_render() const -> void override;

    [[nodiscard]] auto acquire_next_swapchain_target(SwapchainHandle handle) const
        -> ImageHandle override;
    auto present(SwapchainHandle handle, OverlayFunction&& overlay = nullptr) const
        -> void override;
    auto blit_image(ImageHandle source, ImageHandle destination) const -> void override;
    auto read_image(const ImageHandle image) const -> std::vector<u8> override;

    [[nodiscard]] auto limits() const -> const Limits& override;
    [[nodiscard]] auto statistics() const -> Statistics override;
};

} // namespace siren
