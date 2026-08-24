#include "2iren/2iren.hpp"
#include "2iren/rhi/render_target.hpp"
#include "2iren/util/byte_buffer.hpp"

struct Vertex {
    siren::f32 x, y, z;
    siren::f32 r, g, b, a;
};

const siren::ShaderData vertex_shader{
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
const siren::ShaderData fragment_shader{
    .label  = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) in vec4 v_color;

        layout(location = 0) out vec4 FragColor;

        void main() {
            FragColor = v_color;
        })",
};

const std::unordered_map<siren::ShaderStage, siren::ShaderData> shaders = {
    {siren::ShaderStage::Vertex, vertex_shader},
    {siren::ShaderStage::Fragment, fragment_shader},
};

const siren::ByteBuffer vertices{
    Vertex{.x = 0.0f, .y = 0.5f, .z = 0.0f, .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = -0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = 0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f},
};

auto main() -> siren::i32 {
    // init siren
    auto ctx = siren::Context::create(
        {
            .debug   = true,
            .level   = siren::log::Level::Trace,
            .backend = siren::Backend::Auto,
        }
    );
    auto window = ctx.create_window({});

    const auto device    = ctx.create_device({.window = window});
    const auto swapchain = device->create_swapchain(
        {
            .label  = std::nullopt,
            .vsync  = true,
            .extent = glm::uvec2{window.width(), window.height()},
            .window = &window,
        }
    );

    const auto buffer = device->create_buffer(
        {
            .label = std::nullopt,
            .data  = vertices.data(),
            .size  = vertices.size_bytes(),
            .usage = siren::BufferUsage::Static,
        }
    );
    const auto layout = siren::LayoutBuilder::create()
                        .add(siren::Attribute::Position, 3, siren::DataType::Float32)
                        .add(siren::Attribute::Color, 4, siren::DataType::Float32)
                        .finish();

    const auto shader   = device->create_shader({.label = std::nullopt, .source = shaders});
    const auto pipeline = device->create_graphics_pipeline(
        {
            .label             = std::nullopt,
            .layout            = layout,
            .shader            = shader.handle(),
            .topology          = siren::PrimitiveTopology::Triangles,
            .alpha_mode        = siren::AlphaMode::Opaque,
            .depth_function    = siren::DepthFunction::Less,
            .back_face_culling = false,
            .depth_test        = false,
            .depth_write       = true,
        }
    );

    const auto color = device->create_image(
        {
            .format        = siren::ImageFormat::RGBA8,
            .extent        = siren::ImageExtent{.width = window.width(), .height = window.height()},
            .dimension     = siren::ImageDimension::D2,
            .mipmap_levels = 1,
        }
    );
    const siren::RenderTarget target{
        .colors = {
            {
                .image           = color.handle(),
                .begin_operation = siren::BeginOperation::Clear,
                .clear_color     = siren::Rgba::black()
            },
        },
        .depth_stencil = std::nullopt
    };

    while (!window.should_close()) {
        window.poll_events();

        device->render_pass(
            {.target = target},
            [&](siren::RenderPassRecorder& pass) -> void {
                pass.bind_graphics_pipeline(pipeline.handle());
                pass.bind_vertex_buffer(buffer.handle(), 0, 0);
                pass.draw_fullscreen();
            }
        );

        device->blit_image(target.colors[0].image, swapchain.next_image());
        swapchain.present();
        device->flush_delete_queue();
    }

    device->wait_until_idle();
    return 0;
}
