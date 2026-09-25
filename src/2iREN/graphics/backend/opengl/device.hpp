#pragma once

#include <GLFW/glfw3.h>

#include "2iREN/graphics/backend/opengl/resource_state.hpp"
#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/graphics/backend/opengl/opengl.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"

namespace siren {


class OpenGLDevice final : public Device {
public:
    explicit OpenGLDevice();
    ~OpenGLDevice() override;

    [[nodiscard]]
    auto make_buffer(const BufferDescriptor& descriptor, std::optional<ByteBufferView> initial)
        -> Buffer override;

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
    auto make_swapchain(const Window& window, const SwapchainDescriptor& descriptor)
        -> Swapchain override;

    auto reconfigure_swapchain(SwapchainHandle handle, const SwapchainDescriptor& new_values)
        -> void override;

    [[nodiscard]]
    auto make_query(const QueryDescriptor& descriptor) -> Query override;

    auto destroy_buffer(BufferHandle handle) -> void override;

    auto destroy_image(ImageHandle handle) -> void override;

    auto destroy_sampler(SamplerHandle handle) -> void override;

    auto destroy_shader(ShaderHandle handle) -> void override;

    auto destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void override;

    auto destroy_swapchain(SwapchainHandle handle) -> void override;

    auto destroy_query(QueryHandle handle) -> void override;

    [[nodiscard]]
    auto buffer_descriptor(BufferHandle handle) const -> const BufferDescriptor& override;

    [[nodiscard]]
    auto image_descriptor(ImageHandle handle) const -> const ImageDescriptor& override;

    [[nodiscard]]
    auto sampler_descriptor(SamplerHandle handle) const -> const SamplerDescriptor& override;

    [[nodiscard]]
    auto shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& override;

    [[nodiscard]]
    auto graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
        -> const GraphicsPipelineDescriptor& override;

    [[nodiscard]]
    auto swapchain_info(SwapchainHandle handle) const -> const SwapchainInfo& override;

    [[nodiscard]]
    auto query_descriptor(QueryHandle handle) const -> const QueryDescriptor& override;

    [[nodiscard]]
    auto make_command_buffer() const noexcept -> std::unique_ptr<siren::CommandBuffer> override;

    auto submit(std::unique_ptr<CommandBuffer>&& command_buffer) const -> void override;

    auto present(SwapchainHandle handle) -> void override;

    auto present(SwapchainHandle handle, std::unique_ptr<CommandBuffer>&& command_buffer)
        -> void override;

    [[nodiscard]]
    auto acquire_next_swapchain_image(SwapchainHandle handle) -> ImageHandle override;

    [[nodiscard]]
    auto read_buffer(BufferHandle buffer) const -> ByteBuffer override;

    auto wait_idle() const noexcept -> void override;

private:
    opengl::ResourceState m_state;
};

} // namespace siren
