#include <optional>
#include <unordered_map>

#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/core/context.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/graphics/types.hpp"
#include "2iREN/utility/log.hpp"
#include "2iREN/window/window.hpp"

using namespace siren;

#ifdef SIREN_MACOS
const auto shader_source = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float4 color;
};

struct VertexIn {
    float2 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

vertex auto vmain(VertexIn in [[stage_in]]) -> VertexOut {
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.color = in.color;
    return out;
}

fragment auto fmain(VertexOut in [[stage_in]]) -> float4 {
    return in.color;
}
)";
const auto vertex_shader = ShaderData{
    .label  = "Triangle Vertex Shader",
    .source = shader_source,
    .entry  = "vmain",
};
const auto fragment_shader = ShaderData{
    .label  = "Triangle Fragment Shader",
    .source = shader_source,
    .entry  = "fmain",
};
#else
const ShaderData vertex_shader{
    .label  = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) in vec3 a_pos;
        layout(location = 1) in vec4 a_color;

        layout(location = 0) out vec4 v_color;

        void main() {
            gl_Position = vec4(a_pos, 0.0, 1.0);
            v_color = a_color;
        })",
};
const ShaderData fragment_shader{
    .label  = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) in vec4 v_color;

        layout(location = 0) out vec4 FragColor;

        void main() {
            FragColor = v_color;
        })",
};
#endif

const auto shaders = std::unordered_map<ShaderStage, ShaderData>{
    {ShaderStage::Vertex, vertex_shader},
    {ShaderStage::Fragment, fragment_shader},
};

struct Vertex {
    Vec2f pos;
    Rgba color;
};

const auto vertices = ByteBuffer{
    Vertex{Vec2f{0.0f, 0.5f}, Rgba::RED()},
    Vertex{Vec2f{-0.5f, -0.5f}, Rgba::BLUE()},
    Vertex{Vec2f{0.5f, -0.5f}, Rgba::GREEN()},
};

auto main() -> i32 {
    auto ctx             = Context::make({.level = log::Level::Trace});
    auto window          = ctx.make_window({.title = "Example 01"});
    const auto device    = ctx.make_device();
    const auto swapchain = device->make_swapchain(
        window,
        {
            .extent = window.framebuffer_extent(),
            .vsync  = true,
        }
    );

    const auto buffer = device->make_buffer(
        {
            .label        = "Vertex Buffer",
            .size         = vertices.size_bytes(),
            .usage        = BufferFlag::Vertex | BufferFlag::Vertex | BufferFlag::Vertex,
            .memory_usage = MemoryUsage::CpuAndGpu,
        },
        vertices.view()
    );
    const auto
        layout = LayoutBuilder::make().add(DataType::Float32, 2).add(DataType::Float32, 4).finish();

    const auto shader = device->make_shader({.label = "Triangle Shader", .source = shaders});

    const auto attachment = device->make_image({
        .label         = "Color Attachment",
        .format        = swapchain.info().image_format,
        .extent        = swapchain.info().extent.to_extent3(),
        .dimension     = ImageDimension::D2,
        .mipmap_levels = 1,
        .memory_usage  = MemoryUsage::CpuAndGpu,
        .flags         = ImageFlags::make(ImageFlag::RenderAttachment),
    });

    const auto pipeline = device->make_graphics_pipeline({
        .label    = "Triangle Pipeline",
        .shader   = shader.handle(),
        .layout   = layout,
        .topology = PrimitiveTopology::Triangles,
        .colors =
            ColorAttachmentDescriptors{
                ColorAttachmentDescriptor{
                    .format      = swapchain.info().image_format,
                    .alpha_mode  = AlphaMode::Opaque,
                    .color_blend = {},
                    .alpha_blend = {},
                },
            },
        .depth_stencil = std::nullopt,
        .cull_mode     = CullMode::Back,
    });

    log::info("{}", BufferFlag::Index | BufferFlag::Vertex);

    while (!window.should_close()) {
        window.poll_events();

        const auto backbuffer = swapchain.next_image();

        auto cmds = device->make_command_buffer();

        cmds->render_pass(
            RenderPassDescriptor{
                .label = "Triangle Pass",
                .target =
                    RenderTarget{
                        .colors =
                            TargetColorAttachments{
                                TargetColorAttachment{
                                    .image           = backbuffer,
                                    .clear_color     = Rgba::lerp(Rgba::WHITE(), Rgba::RED()),
                                    .begin_operation = BeginOperation::Clear,
                                    .end_operation   = EndOperation::Store,
                                },
                            },
                        .depth_stencil = std::nullopt,
                    },
            },
            [&](RenderCommandEncoder& pass) {
                pass.bind_graphics_pipeline(pipeline.handle());
                pass.bind_vertex_buffer(buffer.handle(), Slot{0});
                pass.draw(3);
            }
        );

        swapchain.present(std::move(cmds));
    }

    return 0;
}
