#include "2iREN/context.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/image.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/render_target.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/utility/byte_buffer.hpp"
#include "2iREN/utility/log.hpp"
#include "2iREN/window.hpp"

using namespace siren;

struct Vertex {
    f32 x, y, z;
    f32 r, g, b, a;
};

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

const std::unordered_map<ShaderStage, ShaderData> shaders = {
    {ShaderStage::Vertex, vertex_shader},
    {ShaderStage::Fragment, fragment_shader},
};

const ByteBuffer vertices{
    Vertex{.x = 0.0f, .y = 0.5f, .z = 0.0f, .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = -0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = 0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f},
};

auto main() -> i32 {
    auto ctx    = Context::create({
        .debug   = true,
        .level   = log::Level::Trace,
        .backend = Backend::Auto,
    });
    auto window = ctx.create_window({});

    const auto device    = ctx.create_device({.window = window});
    const auto swapchain = device->create_swapchain({
        .label  = std::nullopt,
        .vsync  = true,
        .extent = window.extent(),
        .window = &window,
    });

    const auto buffer = device->create_buffer({
        .label = std::nullopt,
        .data  = vertices.data(),
        .size  = vertices.size_bytes(),
        .usage = BufferUsage::Static,
    });
    const auto layout = LayoutBuilder::create()
                            .add(Attribute::Position, 3, DataType::Float32)
                            .add(Attribute::Color, 4, DataType::Float32)
                            .finish();

    const auto shader   = device->create_shader({.label = std::nullopt, .source = shaders});
    const auto pipeline = device->create_graphics_pipeline({
        .label             = std::nullopt,
        .layout            = layout,
        .shader            = shader.handle(),
        .topology          = PrimitiveTopology::Triangles,
        .alpha_mode        = AlphaMode::Opaque,
        .depth_function    = DepthFunction::Less,
        .back_face_culling = false,
        .depth_test        = false,
        .depth_write       = true,
    });

    const auto color = device->create_image({
        .format        = ImageFormat::RGBA8,
        .extent        = Extent3u{window.width(), window.height(), 1},
        .dimension     = ImageDimension::D2,
        .mipmap_levels = 1,
    });
    const RenderTarget target{
        .colors =
            {
                {
                    .image           = color.handle(),
                    .begin_operation = BeginOperation::Clear,
                    .clear_color     = Rgba::BLACK(),
                },
            },
        .depth_stencil = std::nullopt
    };

    while (!window.should_close()) {
        window.poll_events();

        device->render_pass({.target = target}, [&](RenderPassRecorder& pass) -> void {
            pass.bind_graphics_pipeline(pipeline.handle());
            pass.bind_vertex_buffer(buffer.handle(), 0, 0);
            pass.draw_fullscreen();
        });

        device->blit_image(target.colors[0].image, swapchain.next_image());
        swapchain.present();
        device->flush_delete_queue();
    }

    return 0;
}
