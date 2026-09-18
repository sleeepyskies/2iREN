#include "device.hpp"

#include <Foundation/Foundation.hpp>
#include <Metal/MTL4PipelineState.hpp>
#include <Metal/MTLPipeline.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <cstddef>
#include <utility>
#include <version>

#include "2iREN/graphics/backend/metal/command_executor.hpp"
#include "2iREN/graphics/fwd.hpp"

#include "2iREN/core/assert.hpp"
#include "2iREN/graphics/backend/metal/adapter.hpp"
#include "2iREN/graphics/backend/metal/mappings.hpp"
#include "2iREN/graphics/backend/metal/util.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/limits.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/utility/log.hpp"
#include "2iREN/window/window.hpp"

// TODO: impl this and remove flags!

#undef MIN
#undef MAX

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

namespace siren {

namespace {

auto fetch_limits(MTL::Device* device) -> Limits {
    // TODO: what limits can we get? should we remove some? generalize them?

    Limits lim;

    /// lim.max_uniform_buffer_bindings;
    /// lim.max_shader_storage_buffer_bindings;
    /// lim.max_uniform_block_size;
    /// lim.max_shader_storage_block_size;
    /// lim.uniform_buffer_offset_alignment;
    /// lim.shader_storage_buffer_offset_alignment;
    /// lim.max_vertex_attributes;
    /// lim.max_texture_size;
    /// lim.max_array_texture_layers;
    /// lim.max_texture_units;
    /// lim.max_color_attachments;
    /// lim.max_draw_buffers;
    /// lim.max_samples;
    /// lim.max_compute_work_group_invocations;
    /// lim.max_compute_work_group_count;
    /// lim.max_compute_work_group_size;

    return lim;
}

} // namespace

MetalDevice::MetalDevice() : Device(Backend::Metal) {
    m_device    = metal::transfer_ptr(MTL::CreateSystemDefaultDevice());
    m_cmd_queue = metal::transfer_ptr(m_device->newCommandQueue());

    m_limits = fetch_limits(m_device.get());

    log::info("metal device created.");
}

MetalDevice::~MetalDevice() { }

auto MetalDevice::wait_idle() const noexcept -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::make_buffer(
    const BufferDescriptor&       descriptor,
    std::optional<ByteBufferView> initial
) -> Buffer {
    if (initial.has_value()) {
        ASSERT(
            initial->size() <= descriptor.size,
            "buffer size is not large enough to hold the requested data."
        );
    }

    const auto autorelease = metal::AutoRelease{};
    auto       buffer      = (NS::SharedPtr<MTL::Buffer>)nullptr;

    switch (descriptor.usage) {
        // for Static buffers, we must create a staging buffer, then blit its
        // data to a buffer with Private flag
        case BufferUsage::Static: {
            ASSERT(initial.has_value(), "should not create a static buffer with no initial data.");
            auto staging = metal::transfer_ptr(m_device->newBuffer(
                initial->data(), descriptor.size, MTL::ResourceStorageModeShared
            ));
            buffer       = metal::transfer_ptr(
                m_device->newBuffer(descriptor.size, MTL::ResourceStorageModePrivate)
            );

            // TODO: make this its own function?
            auto* cmdbuffer      = m_cmd_queue->commandBuffer();
            auto  blitdescriptor = metal::transfer_ptr(MTL::BlitPassDescriptor::alloc()->init());
            auto* encoder        = cmdbuffer->blitCommandEncoder(blitdescriptor.get());

            encoder->copyFromBuffer(staging.get(), 0, buffer.get(), 0, initial->size());
            encoder->endEncoding();

            cmdbuffer->commit();
            cmdbuffer->waitUntilCompleted(); // keep buffer alive before we can
                                             // delete it

            break;
        }
        // for Dynamic buffers, we simply create a Shared buffer
        case BufferUsage::Dynamic: {
            if (initial.has_value()) {
                buffer = metal::transfer_ptr(m_device->newBuffer(
                    initial->data(), descriptor.size, MTL::ResourceStorageModeShared
                ));
            } else {
                buffer = metal::transfer_ptr(
                    m_device->newBuffer(descriptor.size, MTL::ResourceStorageModeShared)
                );
            }
            break;
        }
    }

    ASSERT_NOT_NULL(buffer.get(), "failed to create metal buffer.");

    if (descriptor.label) {
        buffer->setLabel(metal::utf8_string(*descriptor.label).get());
    }

    const auto handle =
        m_state.buffers.reserve_link(std::move(buffer), BufferDescriptor{descriptor});

    log::trace("created buffer {}", handle);
    return Buffer{this, handle};
}

auto MetalDevice::destroy_buffer(BufferHandle handle) -> void {
    m_state.buffers.fetch_release(handle);
    log::trace("destroyed buffer {}", handle);
}

auto MetalDevice::make_image(
    const ImageDescriptor&        descriptor,
    std::optional<ByteBufferView> initial
) -> Image {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_image(ImageHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::make_sampler(const SamplerDescriptor& descriptor) -> Sampler {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_sampler(SamplerHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::make_shader(const ShaderDescriptor& descriptor) -> Shader {
    auto* err = (NS::Error*)nullptr;
    ASSERT(
        descriptor.source.contains(ShaderStage::Vertex),
        "cannot create a shader without a vertex shader"
    );
    ASSERT(
        descriptor.source.contains(ShaderStage::Fragment),
        "cannot create a shader without a fragment shader"
    );

    // create shader compiler
    auto compiler_descriptor = metal::transfer_ptr(MTL4::CompilerDescriptor::alloc()->init());
    auto compiler            = m_device->newCompiler(compiler_descriptor.get(), &err);

    metal::check_error(compiler, err);

    // create shader library
    auto compile_opts = metal::transfer_ptr(MTL::CompileOptions::alloc()->init());
    compile_opts->setEnableLogging(true);
    auto source = metal::utf8_string(descriptor.source.at(ShaderStage::Vertex).source);

    auto library_desc = metal::transfer_ptr(MTL4::LibraryDescriptor::alloc()->init());
    library_desc->setSource(source.get());
    library_desc->setOptions(compile_opts.get());
    if (descriptor.label) {
        library_desc->setName(metal::utf8_string(*descriptor.label).get());
    }

    auto library = metal::transfer_ptr(compiler->newLibrary(library_desc.get(), &err));
    metal::check_error(library, err);

    const auto handle =
        m_state.shaders.reserve_link(library, MetalShaderDetails{descriptor, compiler});

    log::trace("created shader {}", handle);

    return Shader{this, handle};
}

auto MetalDevice::destroy_shader(ShaderHandle handle) -> void {
    log::trace("destroyed shader {}", handle);
    m_state.shaders.fetch_release(handle);
}

auto MetalDevice::make_swapchain(const Window& window, const SwapchainDescriptor& descriptor)
    -> Swapchain {
    // in the metal api, the CAMetalLayer acts as th swapchain.
    // this object manages various Drawables, which are swapchain images.

    auto* layer = CA::MetalLayer::layer();

    layer->setDevice(m_device.get());
    layer->setPixelFormat(MTL::PixelFormatBGRA8Unorm); // TODO: what format bgra or rgba
    layer->setDisplaySyncEnabled(descriptor.vsync);

    const auto handle = m_state.swapchains.reserve_link(layer, MetalSwapchainDetails{descriptor});

    metal::connect_to_window(window.native_handle(), layer);

    // TODO: do we want to supply an image format? this does not handle color
    // space mapping for us

    log::trace("created swapchain {}", handle);
    return Swapchain{this, handle};
}

auto MetalDevice::destroy_swapchain(SwapchainHandle handle) -> void {
    m_state.swapchains.release(handle);
    log::trace("destroyed swapchain {}", handle);
}

auto MetalDevice::make_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
    -> GraphicsPipeline {
    auto* err = (NS::Error*)nullptr;

    auto renderpipeline_descriptor =
        metal::transfer_ptr(MTL4::RenderPipelineDescriptor::alloc()->init());

    if (descriptor.label) {
        renderpipeline_descriptor->setLabel(metal::utf8_string(*descriptor.label).get());
    }

    // enable validation always
    {
        auto opts = metal::transfer_ptr(MTL4::PipelineOptions::alloc()->init());
        opts->setShaderValidation(MTL::ShaderValidationEnabled);
        renderpipeline_descriptor->setOptions(opts.get());
    }

    // color attachments
    for (usize i = 0; i < descriptor.colors.size(); i++) {
        const auto& colortarget     = descriptor.colors[i];
        auto* attachment_descriptor = renderpipeline_descriptor->colorAttachments()->object(i);

        attachment_descriptor->setAlphaBlendOperation(
            metal::blend_operation(colortarget.alpha_blend.function)
        );
        attachment_descriptor->setBlendingState(metal::blending_state(colortarget.alpha_mode));

        attachment_descriptor->setDestinationAlphaBlendFactor(
            metal::blend_factor(colortarget.alpha_blend.dest_factor)
        );

        attachment_descriptor->setSourceAlphaBlendFactor(
            metal::blend_factor(colortarget.alpha_blend.source_factor)
        );

        attachment_descriptor->setDestinationRGBBlendFactor(
            metal::blend_factor(colortarget.color_blend.dest_factor)
        );
        attachment_descriptor->setSourceRGBBlendFactor(
            metal::blend_factor(colortarget.color_blend.source_factor)
        );

        attachment_descriptor->setPixelFormat(metal::pixel_format(colortarget.format));
        attachment_descriptor->setRgbBlendOperation(
            metal::blend_operation(colortarget.color_blend.function)
        );
    }

    // depth desciption is done during the render pass.

    // vertex buffer layout
    {
        auto* layout = MTL::VertexDescriptor::alloc()->init();

        for (usize i = 0; i < descriptor.layout.components.size(); i++) {
            auto* vertex    = layout->attributes()->object(i);
            auto& component = descriptor.layout.components[i];
            vertex->setFormat(metal::vertex_format(component));
            vertex->setOffset(component.offset);
            vertex->setBufferIndex(0);
        }

        auto* buf_layout = layout->layouts()->object(0);
        buf_layout->setStepFunction(MTL::VertexStepFunctionPerVertex);
        buf_layout->setStride(descriptor.layout.stride);
        buf_layout->setStepRate(1);

        renderpipeline_descriptor->setVertexDescriptor(layout);
    }

    // link shader
    {
        // TODO: should we migrate code to the more complex MTL4 api?

        auto        library     = m_state.shaders.fetch(descriptor.shader);
        const auto& shader_desc = m_state.shaders.details(descriptor.shader).descriptor;

        auto vertexfn = metal::transfer_ptr(MTL4::LibraryFunctionDescriptor::alloc()->init());
        vertexfn->setLibrary(library.get());
        vertexfn->setName(
            metal::utf8_string(shader_desc.source.at(ShaderStage::Vertex).entry).get()
        );

        auto fragmentfn = metal::transfer_ptr(MTL4::LibraryFunctionDescriptor::alloc()->init());
        fragmentfn->setLibrary(library.get());
        fragmentfn->setName(
            metal::utf8_string(shader_desc.source.at(ShaderStage::Fragment).entry).get()
        );

        renderpipeline_descriptor->setVertexFunctionDescriptor(vertexfn.get());
        renderpipeline_descriptor->setFragmentFunctionDescriptor(fragmentfn.get());
    }

    auto render_pipeline = metal::transfer_ptr(
        m_state.shaders.details(descriptor.shader)
            .compiler->newRenderPipelineState(renderpipeline_descriptor.get(), nullptr, &err)
    );
    metal::check_error(render_pipeline, err);

    const auto handle =
        m_state.pipelines.reserve_link(render_pipeline, GraphicsPipelineDescriptor{descriptor});
    log::trace("created graphics pipeline {}", handle);
    return GraphicsPipeline{this, handle};
}

auto MetalDevice::destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void {
    log::trace("destroyed graphics pipeline {}", handle);
    m_state.pipelines.fetch_release(handle);
}

auto MetalDevice::make_query(const QueryDescriptor& descriptor) -> Query {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_query(QueryHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::submit(CommandList&& cmds) -> void {
    const auto autorelease = metal::AutoRelease{};

    auto cmd_buffer = metal::retain_ptr(m_cmd_queue->commandBuffer());

    auto executor = MetalCommandExecutor{this->m_state, cmd_buffer};
    executor.execute(std::move(cmds));

    cmd_buffer->commit();

    m_statistics += executor.statistics();
}

auto MetalDevice::buffer_descriptor(BufferHandle handle) const -> const BufferDescriptor& {
    return m_state.buffers.details(handle);
}

auto MetalDevice::image_descriptor(ImageHandle handle) const -> const ImageDescriptor& {
    return m_state.images.details(handle);
}

auto MetalDevice::sampler_descriptor(SamplerHandle handle) const -> const SamplerDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::shader_descriptor(ShaderHandle handle) const -> const ShaderDescriptor& {
    return m_state.shaders.details(handle).descriptor;
}

auto MetalDevice::graphics_pipeline_descriptor(GraphicsPipelineHandle handle) const
    -> const GraphicsPipelineDescriptor& {
    return m_state.pipelines.details(handle);
}

auto MetalDevice::swapchain_descriptor(SwapchainHandle handle) const -> const SwapchainDescriptor& {
    return m_state.swapchains.details(handle).descriptor;
}

auto MetalDevice::swapchain_info(SwapchainHandle handle) const -> SwapchainInfo {
    const auto layer = m_state.swapchains.fetch(handle);

    const auto pixel_format = layer->pixelFormat();

    return SwapchainInfo{
        .image_format = metal::image_format(pixel_format),
    };
}

auto MetalDevice::query_descriptor(QueryHandle handle) const -> const QueryDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::query_result(QueryHandle handle) const -> u64 {
    UNIMPLEMENTED();
}

auto MetalDevice::query_available(QueryHandle handle) const -> bool {
    UNIMPLEMENTED();
}

auto MetalDevice::begin_conditional_render(const QueryHandle query) const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::end_conditional_render() const -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::acquire_next_swapchain_image(SwapchainHandle handle) -> ImageHandle {
    auto& details = m_state.swapchains.details(handle);

    if (details.image && details.drawable) {
        return *details.image;
    }

    auto* layer = m_state.swapchains.fetch(handle);

    auto drawable = metal::retain_ptr(layer->nextDrawable());
    auto texture  = drawable->texture();

    details.drawable = drawable;
    const auto size  = layer->drawableSize();

    details.image = m_state.images.reserve_link(
        drawable->texture(),
        ImageDescriptor{
            .label         = "Swapchain Image",
            .format        = metal::image_format(layer->pixelFormat()),
            .extent        = Extent2u{size.width, size.height}.to_extent3(),
            .dimension     = ImageDimension::D2,
            .mipmap_levels = static_cast<u32>(texture->mipmapLevelCount()),
        }
    );

    return *details.image;
}

auto MetalDevice::present(SwapchainHandle handle) -> void {
    auto& details  = m_state.swapchains.details(handle);
    auto  drawable = details.drawable;

    ASSERT_NOT_NULL(drawable.get(), "metal: cannot present swapchain, no drawable present.");

    if (details.drawable) {
        details.drawable->release();
    }
    details.drawable = nullptr;
    if (details.image) {
        m_state.images.release(*details.image);
    }
    details.image = std::nullopt;
}

auto MetalDevice::present(SwapchainHandle handle, CommandList&& cmds) -> void {
    const auto autorelease = metal::AutoRelease{};

    auto& details  = m_state.swapchains.details(handle);
    auto  drawable = details.drawable;

    ASSERT_NOT_NULL(drawable.get(), "metal: cannot present swapchain, no drawable present.");

    auto cmd_buffer = metal::retain_ptr(m_cmd_queue->commandBuffer());

    auto executor = MetalCommandExecutor{this->m_state, cmd_buffer};
    executor.execute(std::move(cmds));

    cmd_buffer->presentDrawable(drawable.get());
    cmd_buffer->commit();

    m_statistics += executor.statistics();

    if (details.image) {
        m_state.images.release(*details.image);
    }
    details.image    = std::nullopt;
    details.drawable = nullptr;
}

} // namespace siren
