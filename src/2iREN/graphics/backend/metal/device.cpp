#include "device.hpp"

#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"

// TODO: impl this and remove flags!

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

namespace siren {

MetalDevice::MetalDevice() {
    UNIMPLEMENTED();
}

MetalDevice::~MetalDevice() {
    UNIMPLEMENTED();
}

auto MetalDevice::wait_idle() const noexcept -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::create_buffer(const BufferDescriptor& descriptor) -> Buffer {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_buffer(BufferHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::create_image(const ImageDescriptor& descriptor) -> Image {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_image(ImageHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::create_sampler(const SamplerDescriptor& descriptor) -> Sampler {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_sampler(SamplerHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::create_shader(const ShaderDescriptor& descriptor) -> Shader {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_shader(ShaderHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::create_swapchain(const SwapchainDescriptor& descriptor) -> Swapchain {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_swapchain(SwapchainHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::create_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
    -> GraphicsPipeline {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::create_query(const QueryDescriptor& descriptor) -> Query {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_query(QueryHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::record_resource_commands() const -> ResourceCommandRecorder {
    UNIMPLEMENTED();
}

auto MetalDevice::record_render_commands() const -> RenderCommandRecorder {
    UNIMPLEMENTED();
}

auto MetalDevice::submit(ResourceCommandBuffer&& command_buffer) const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::submit(RenderCommandBuffer&& command_buffer) const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::buffer_descriptor(BufferHandle handle) const -> const BufferDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::image_descriptor(ImageHandle handle) const -> const ImageDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::sampler_descriptor(SamplerHandle handle) const -> const SamplerDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
    -> const GraphicsPipelineDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::swapchain_descriptor(SwapchainHandle handle) const -> const SwapchainDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::query_descriptor(QueryHandle handle) const -> const QueryDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::query_result(QueryHandle handle) const -> u64 {
    UNIMPLEMENTED();
}

auto MetalDevice::begin_conditional_render(const QueryHandle query) const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::end_conditional_render() const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::acquire_next_swapchain_target(SwapchainHandle handle) const -> ImageHandle {
    UNIMPLEMENTED();
}

auto MetalDevice::present(SwapchainHandle handle, OverlayFunction&& overlay) const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::blit_image(ImageHandle source, ImageHandle destination) const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::read_image(const ImageHandle image) const -> std::vector<u8> {
    UNIMPLEMENTED();
}

auto MetalDevice::limits() const -> const Limits& {
    UNIMPLEMENTED();
}

auto MetalDevice::statistics() const -> Statistics {
    UNIMPLEMENTED();
}

} // namespace siren

#pragma clang diagnostic pop
