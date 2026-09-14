#include "2iREN/core/context.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/render_target.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/utility/byte_buffer.hpp"
#include "2iREN/utility/log.hpp"
#include "2iREN/window/window.hpp"

using namespace siren;

struct Vertex {
    f32 x, y, z;
    f32 r, g, b, a;
};

#ifdef SIREN_MACOS
const auto shader_source = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float4 color;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

vertex VertexOut vmain(VertexIn in [[stage_in]]) {
    VertexOut out;
    out.position = float4(in.position, 1.0);
    out.color = in.color;
    return out;
}

fragment float4 fmain(VertexOut in [[stage_in]]){
    return in.color;
}
)";
const ShaderData vertex_shader{
    .label  = "Triangle Vertex Shader",
    .source = shader_source,
    .entry  = "vmain",
};
const ShaderData fragment_shader{
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
            gl_Position = vec4(a_pos, 1.0);
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

const std::unordered_map<ShaderStage, ShaderData> shaders = {
    {ShaderStage::Vertex, vertex_shader},
    {ShaderStage::Fragment, fragment_shader},
};

const ByteBuffer vertices{
    Vertex{0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f},
    Vertex{-0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
    Vertex{0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f},
};

auto main() -> i32 {
    auto ctx             = Context::make({
        .debug = true,
        .level = log::Level::Trace,
    });
    auto window          = ctx.make_window({.title = "Example 01"});
    const auto device    = ctx.make_device();
    const auto swapchain = device->make_swapchain(window, {.vsync = true});

    const auto buffer = device->make_buffer(
        {
            .label = std::nullopt,
            .size  = vertices.size_bytes(),
            .usage = BufferUsage::Static,
        },
        vertices.view()
    );
    const auto layout = LayoutBuilder::create()
                            .add(Attribute::Position, 3, DataType::Float32)
                            .add(Attribute::Color, 4, DataType::Float32)
                            .finish();

    const auto shader = device->make_shader({
        .label  = "Triangle Shader",
        .source = shaders,
    });

    const auto pipeline = device->make_graphics_pipeline({
        .label             = "Triagle Pipeline",
        .layout            = layout,
        .shader            = shader.handle(),
        .alpha_mode        = AlphaMode::Opaque,
        .depth_function    = DepthFunction::Less,
        .back_face_culling = false,
        .depth_test        = false,
        .depth_write       = true,
    });

    auto pass = RenderPassDescriptor{
        .label  = "Triangle Render Pass",
        .target = RenderTarget{
            .colors = ColorAttachments{
                ColorAttachment{
                    .image           = swapchain.next_image(),
                    .begin_operation = BeginOperation::Clear,
                    .clear_color     = Rgba::BLACK(),
                },
            },
        },
    };

    while (!window.should_close()) {
        window.poll_events();

        pass.target.colors[0].image = swapchain.next_image();

        device->render_pass(pass, [&](RenderPassRecorder& pass) -> void {
            pass.bind_graphics_pipeline(pipeline.handle());
            pass.bind_vertex_buffer(buffer.handle(), 0, 0);
            pass.draw_arrays(PrimitiveTopology::Triangles, 0, 3);
        });

        swapchain.present();
    }

    return 0;
}
