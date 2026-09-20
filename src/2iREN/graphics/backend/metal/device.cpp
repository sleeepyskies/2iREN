#include "device.hpp"

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

#include <cstring>
#include <memory>

#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/core/base.hpp"
#include "2iREN/graphics/backend/metal/commands.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/fwd.hpp"

#include "2iREN/core/assert.hpp"
#include "2iREN/graphics/backend/metal/adapter.hpp"
#include "2iREN/graphics/backend/metal/mappings.hpp"
#include "2iREN/graphics/backend/metal/util.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/limits.hpp"
#include "2iREN/graphics/query.hpp"
#include "2iREN/graphics/sampler.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/math/extent.hpp"
#include "2iREN/utility/log.hpp"
#include "2iREN/window/window.hpp"

// TODO: impl this and remove flags!

#undef MIN
#undef MAX

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

namespace siren {

using namespace metal;

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
    m_device    = transfer_ptr(MTL::CreateSystemDefaultDevice());
    m_cmd_queue = transfer_ptr(m_device->newCommandQueue());

    m_limits = fetch_limits(m_device.get());

    log::info("metal device created.");
}

MetalDevice::~MetalDevice() {
    // TODO: this will crash if called
    // wait_idle();
}

auto MetalDevice::wait_idle() const noexcept -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::make_buffer(
    const BufferDescriptor& descriptor,
    std::optional<ByteBufferView> initial
) -> Buffer {
    AUTORELEASE {
        ASSERT(
            initial.transform(&ByteBufferView::size).value_or(0) <= descriptor.size.get(),
            "buffer size is not large enough to hold the requested data."
        );
        auto buffer = transfer_ptr(
            m_device->newBuffer(descriptor.size, resource_options(descriptor.memory_usage))
        );

        set_label(buffer, descriptor.label);

        if (initial.has_value()) {
            switch (descriptor.memory_usage) {
                case MemoryUsage::CpuAndGpu: {
                    bufcpy(*initial, buffer->contents());
                    break;
                }
                case MemoryUsage::GpuOnly: {
                    auto staging = transfer_ptr(m_device->newBuffer(
                        initial->data(), initial->size(), MTL::ResourceStorageModeShared
                    ));

                    auto blitdescriptor = transfer_ptr(MTL::BlitPassDescriptor::alloc()->init());

                    auto cmdbuffer = m_cmd_queue->commandBuffer();
                    auto encoder   = cmdbuffer->blitCommandEncoder(blitdescriptor.get());

                    encoder->copyFromBuffer(staging.get(), 0, buffer.get(), 0, initial->size());
                    encoder->endEncoding();

                    cmdbuffer->commit();
                    cmdbuffer->waitUntilCompleted();
                    check(cmdbuffer);
                    break;
                }
            }
        }

        const auto handle =
            m_state.buffers.reserve_link(std::move(buffer), BufferDescriptor{descriptor});

        log::trace("created buffer {}", handle);
        return Buffer{this, handle};
    };
}

auto MetalDevice::destroy_buffer(BufferHandle handle) -> void {
    m_state.buffers.fetch_release(handle);
    log::trace("destroyed buffer {}", handle);
}

auto MetalDevice::make_image(
    const ImageDescriptor& descriptor,
    std::optional<ByteBufferView> initial
) -> Image {
    AUTORELEASE {
        // TODO: cannot upload direct to private buffers, need staging for that
        auto texture_desc = transfer_ptr(MTL::TextureDescriptor::alloc()->init());

        texture_desc->setTextureType(texture_type(descriptor.dimension));
        texture_desc->setPixelFormat(pixel_format(descriptor.format));
        texture_desc->setWidth(descriptor.extent.x);
        texture_desc->setHeight(descriptor.extent.y);
        texture_desc->setMipmapLevelCount(descriptor.mipmap_levels);
        texture_desc->setResourceOptions(resource_options(descriptor.memory_usage));
        texture_desc->setUsage(texture_usage(descriptor.flags));

        auto texture = transfer_ptr(m_device->newTexture(texture_desc.get()));
        set_label(texture, descriptor.label);

        if (initial.has_value()) {
            const auto bytes_per_row = descriptor.extent.x * descriptor.format.bytes_per_pixel();
            texture->replaceRegion(region(descriptor.extent), 0, initial->data(), bytes_per_row);
        }

        const auto handle = m_state.images.reserve_link(texture, ImageDescriptor{descriptor});

        log::trace("created image {}", handle);
        return Image{this, handle};
    }
}

auto MetalDevice::destroy_image(ImageHandle handle) -> void {
    m_state.images.fetch_release(handle);
    log::trace("destroyed image {}", handle);
}

auto MetalDevice::make_sampler(const SamplerDescriptor& descriptor) -> Sampler {
    auto mtl_desc = transfer_ptr(MTL::SamplerDescriptor::alloc()->init());

    mtl_desc->setRAddressMode(address_mode(descriptor.r_wrap));
    mtl_desc->setSAddressMode(address_mode(descriptor.s_wrap));
    mtl_desc->setTAddressMode(address_mode(descriptor.t_wrap));
    // mtl_desc->setBorderColor(descriptor.border_color);

    mtl_desc->setMinFilter(minmag_filter(descriptor.min_filter));
    mtl_desc->setMagFilter(minmag_filter(descriptor.mag_filter));
    // mtl_desc->setMipFilter(mipmap_filter(descriptor.mipmap_filter));
    // mtl_desc->setLodMinClamp(); mtl_desc->setLodMaxClamp();
    // mtl_desc->setLodAverage();
    // mtl_desc->setMaxAnisotropy();

    // mtl_desc->setCompareFunction();

    // mtl_desc->setLodBias();
    // mtl_desc->setReductionMode();

    set_label(mtl_desc, descriptor.label);
    auto sampler      = transfer_ptr(m_device->newSamplerState(mtl_desc.get()));
    const auto handle = m_state.samplers.reserve_link(sampler, SamplerDescriptor{descriptor});

    log::trace("created sampler {}", handle);
    return Sampler{this, handle};
}

auto MetalDevice::destroy_sampler(SamplerHandle handle) -> void {
    m_state.samplers.fetch_release(handle);
    log::trace("destroyed sampler {}", handle);
}

auto MetalDevice::make_shader(const ShaderDescriptor& descriptor) -> Shader {
    AUTORELEASE {
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
        auto compiler_descriptor = transfer_ptr(MTL4::CompilerDescriptor::alloc()->init());
        auto compiler            = m_device->newCompiler(compiler_descriptor.get(), &err);

        check_error(compiler, err);

        // create shader library
        auto compile_opts = transfer_ptr(MTL::CompileOptions::alloc()->init());
        compile_opts->setEnableLogging(true);

        auto library_desc = transfer_ptr(MTL4::LibraryDescriptor::alloc()->init());
        library_desc->setSource(utf8_string(descriptor.source.at(ShaderStage::Vertex).source));
        library_desc->setOptions(compile_opts.get());
        if (descriptor.label) {
            library_desc->setName(utf8_string(*descriptor.label));
        }

        auto library = transfer_ptr(compiler->newLibrary(library_desc.get(), &err));
        check_error(library, err);

        const auto handle =
            m_state.shaders.reserve_link(library, ShaderDetails{descriptor, compiler});

        log::trace("created shader {}", handle);

        return Shader{this, handle};
    }
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

    const auto extent = descriptor.extent.value_or(window.framebuffer_extent());

    layer->setDrawableSize(CGSize{static_cast<CGFloat>(extent.x), static_cast<CGFloat>(extent.y)});
    layer->setDisplaySyncEnabled(descriptor.vsync.value_or(true));
    layer->setPixelFormat(pixel_format(descriptor.image_format.value_or(ImageFormat::BGRA8)));

    const auto handle = m_state.swapchains.reserve_link(
        layer,
        SwapchainDetails{
            .descriptor = {
                .extent       = extent,
                .vsync        = layer->displaySyncEnabled(),
                .image_format = image_format(layer->pixelFormat()),
            },
        }
    );

    connect_to_window(window.native_handle(), layer);

    log::trace("created swapchain {}", handle);
    return Swapchain{this, handle};
}

auto MetalDevice::update_swapchain(SwapchainHandle handle, const SwapchainDescriptor& new_values)
    -> void {
    auto* layer = m_state.swapchains.fetch(handle);
    auto& old   = m_state.swapchains.details(handle).descriptor;

    if (new_values.extent) {
        const auto& newsize = new_values.extent.value();
        old.extent          = newsize;
        layer->setDrawableSize(
            CGSize{static_cast<CGFloat>(newsize.x), static_cast<CGFloat>(newsize.y)}
        );
    }

    if (new_values.vsync) {
        old.vsync = new_values.vsync.value();
        layer->setDisplaySyncEnabled(new_values.vsync.value());
    }

    if (new_values.image_format) {
        old.vsync = new_values.image_format.value();
        layer->setPixelFormat(pixel_format(new_values.image_format.value()));
    }

    log::trace("updated swapchain {}", handle);
}

auto MetalDevice::destroy_swapchain(SwapchainHandle handle) -> void {
    m_state.swapchains.release(handle);
    log::trace("destroyed swapchain {}", handle);
}

auto MetalDevice::make_graphics_pipeline(const GraphicsPipelineDescriptor& descriptor)
    -> GraphicsPipeline {
    AUTORELEASE {
        auto* err = (NS::Error*)nullptr;

        auto renderpipeline_descriptor =
            transfer_ptr(MTL4::RenderPipelineDescriptor::alloc()->init());

        if (descriptor.label) {
            renderpipeline_descriptor->setLabel(utf8_string(*descriptor.label));
        }

        // enable validation always
        {
            auto opts = transfer_ptr(MTL4::PipelineOptions::alloc()->init());
            opts->setShaderValidation(MTL::ShaderValidationEnabled);
            renderpipeline_descriptor->setOptions(opts.get());
        }

        // color attachments
        for (usize i = 0; i < descriptor.colors.size(); i++) {
            const auto& colortarget     = descriptor.colors[i];
            auto* attachment_descriptor = renderpipeline_descriptor->colorAttachments()->object(i);

            attachment_descriptor->setAlphaBlendOperation(
                blend_operation(colortarget.alpha_blend.function)
            );
            attachment_descriptor->setBlendingState(blending_state(colortarget.alpha_mode));

            attachment_descriptor->setDestinationAlphaBlendFactor(
                blend_factor(colortarget.alpha_blend.dest_factor)
            );

            attachment_descriptor->setSourceAlphaBlendFactor(
                blend_factor(colortarget.alpha_blend.source_factor)
            );

            attachment_descriptor->setDestinationRGBBlendFactor(
                blend_factor(colortarget.color_blend.dest_factor)
            );
            attachment_descriptor->setSourceRGBBlendFactor(
                blend_factor(colortarget.color_blend.source_factor)
            );

            attachment_descriptor->setPixelFormat(pixel_format(colortarget.format));
            attachment_descriptor->setRgbBlendOperation(
                blend_operation(colortarget.color_blend.function)
            );
        }

        // depth desciption is done during the render pass.

        // vertex buffer layout
        {
            auto* layout = MTL::VertexDescriptor::alloc()->init();

            for (usize i = 0; i < descriptor.layout.components.size(); i++) {
                auto* vertex    = layout->attributes()->object(i);
                auto& component = descriptor.layout.components[i];
                vertex->setFormat(vertex_format(component));
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

            auto library            = m_state.shaders.fetch(descriptor.shader);
            const auto& shader_desc = m_state.shaders.details(descriptor.shader).descriptor;

            auto vertexfn = transfer_ptr(MTL4::LibraryFunctionDescriptor::alloc()->init());
            vertexfn->setLibrary(library.get());
            vertexfn->setName(utf8_string(shader_desc.source.at(ShaderStage::Vertex).entry));

            auto fragmentfn = transfer_ptr(MTL4::LibraryFunctionDescriptor::alloc()->init());
            fragmentfn->setLibrary(library.get());
            fragmentfn->setName(utf8_string(shader_desc.source.at(ShaderStage::Fragment).entry));

            renderpipeline_descriptor->setVertexFunctionDescriptor(vertexfn.get());
            renderpipeline_descriptor->setFragmentFunctionDescriptor(fragmentfn.get());
        }

        auto render_pipeline = transfer_ptr(
            m_state.shaders.details(descriptor.shader)
                .compiler->newRenderPipelineState(renderpipeline_descriptor.get(), nullptr, &err)
        );
        check_error(render_pipeline, err);

        const auto handle =
            m_state.pipelines.reserve_link(render_pipeline, GraphicsPipelineDescriptor{descriptor});
        log::trace("created graphics pipeline {}", handle);
        return GraphicsPipeline{this, handle};
    }
}

auto MetalDevice::destroy_graphics_pipeline(GraphicsPipelineHandle handle) -> void {
    log::trace("destroyed graphics pipeline {}", handle);
    m_state.pipelines.fetch_release(handle);
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

auto MetalDevice::swapchain_info(SwapchainHandle handle) const -> const SwapchainInfo& {
    auto& info       = m_state.swapchains.details(handle).descriptor;
    const auto layer = m_state.swapchains.fetch(handle);

    const auto size   = layer->drawableSize();
    info.image_format = image_format(layer->pixelFormat());
    info.extent       = Extent2u{size.width, size.height};
    info.vsync        = layer->displaySyncEnabled();

    return info;
}

auto MetalDevice::make_query(const QueryDescriptor& descriptor) -> Query {
    UNIMPLEMENTED();
}

auto MetalDevice::destroy_query(const QueryHandle handle) -> void {
    UNIMPLEMENTED();
}

auto MetalDevice::query_descriptor(QueryHandle handle) const -> const QueryDescriptor& {
    UNIMPLEMENTED();
}

auto MetalDevice::make_command_buffer() const noexcept -> std::unique_ptr<::siren::CommandBuffer> {
    auto* cmdbuffer = m_cmd_queue->commandBuffer();
    return std::make_unique<metal::CommandBuffer>(cmdbuffer, m_state);
}

auto MetalDevice::submit(std::unique_ptr<siren::CommandBuffer>&& command_buffer) const -> void {
    auto* mtlbuffer = static_cast<metal::CommandBuffer&>(*command_buffer).mtl_command_buffer();
    mtlbuffer->commit();
    check(mtlbuffer);
}

auto MetalDevice::acquire_next_swapchain_image(SwapchainHandle handle) -> ImageHandle {
    auto& details = m_state.swapchains.details(handle);

    if (details.image && details.drawable) {
        return *details.image;
    }

    auto* layer = m_state.swapchains.fetch(handle);

    auto drawable = retain_ptr(layer->nextDrawable());
    auto texture  = drawable->texture();

    details.drawable = drawable;
    const auto size  = layer->drawableSize();

    details.image = m_state.images.reserve_link(
        retain_ptr(drawable->texture()),
        ImageDescriptor{
            .label         = "swapchain image",
            .format        = image_format(layer->pixelFormat()),
            .extent        = Extent2u{size.width, size.height}.to_extent3(),
            .dimension     = ImageDimension::D2,
            .mipmap_levels = static_cast<u32>(texture->mipmapLevelCount()),
            .flags         = ImageFlags::empty(), // flags dont acc matter here.
        }
    );

    return *details.image;
}

auto MetalDevice::read_buffer(BufferHandle buffer) const -> ByteBuffer {
    auto* buf = m_state.buffers.fetch(buffer).get();
    ASSERT_NOT_NULL(buf);

    ByteBuffer output;
    output.reserve_bytes(buf->length());
    std::memcpy(output.data(), buf->contents(), buf->length());

    return output;
}

auto MetalDevice::present(SwapchainHandle handle) -> void {
    auto& details = m_state.swapchains.details(handle);
    auto drawable = details.drawable;

    ASSERT_NOT_NULL(drawable.get(), "cannot present swapchain, no drawable present.");

    drawable->present();

    if (details.drawable) {
        details.drawable->release();
    }
    details.drawable = nullptr;
    if (details.image) {
        m_state.images.release(*details.image);
    }
    details.image = std::nullopt;
}

auto MetalDevice::present(
    SwapchainHandle handle,
    std::unique_ptr<siren::CommandBuffer>&& command_buffer
) -> void {
    auto* mtlbuffer = static_cast<metal::CommandBuffer&>(*command_buffer).mtl_command_buffer();

    auto& details = m_state.swapchains.details(handle);
    auto drawable = details.drawable;

    mtlbuffer->presentDrawable(drawable.get());
    mtlbuffer->commit();

    ASSERT_NOT_NULL(drawable.get(), "cannot present swapchain, no drawable present.");

    if (details.drawable) {
        details.drawable->release();
    }
    details.drawable = nullptr;
    if (details.image) {
        m_state.images.release(*details.image);
    }
    details.image = std::nullopt;
}

} // namespace siren
