#include "2iren/2iren.hpp"
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
    { siren::ShaderStage::Vertex, vertex_shader },
    { siren::ShaderStage::Fragment, fragment_shader },
};
const siren::ByteBuffer vertices{
    Vertex{ .x = 0.0f, .y = 0.5f, .z = 0.0f, .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f },
    Vertex{ .x = -0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f },
    Vertex{ .x = 0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f },
};

auto main() -> siren::i32 {
    // init siren
    siren::Context ctx{ { .debug = true, .level = siren::log::Level::Trace, .backend = siren::Backend::Auto } };

    siren::Window window({
            .title       = "2iren",
            .width       = 1280,
            .height      = 800,
            .fullscreen  = false,
            .vsync       = true,
            .decorated   = true,
            .resizable   = true,
            .transparent = false,
    });

    auto device    = ctx.create_device(window);
    auto swapchain = device->create_swapchain({
            .label = std::nullopt,
            .vsync = true,
    });

    const auto buffer = device->create_buffer({
            .label = "sample_buffer",
            .data  = vertices.data(),
            .size  = vertices.size_bytes(),
            .usage = siren::BufferUsage::Static,
    });
    const auto layout = siren::LayoutBuilder::start()
                                .add(siren::Attribute::Position, 3, siren::DataType::Float32)
                                .add(siren::Attribute::Color, 4, siren::DataType::Float32)
                                .finish();

    const auto shader   = device->create_shader({
              .label  = std::nullopt,
              .source = shaders,
    });
    const auto pipeline = device->create_graphics_pipeline({
            .label             = std::nullopt,
            .layout            = layout,
            .shader            = shader.handle(),
            .topology          = siren::PrimitiveTopology::Triangles,
            .alpha_mode        = siren::AlphaMode::Opaque,
            .depth_function    = siren::DepthFunction::Less,
            .back_face_culling = false,
            .depth_test        = false,
            .depth_write       = true,
    });

    while (!window.should_close()) {
        window.poll_events();

        auto cmds = device->record_render_commands();
        {
            auto pass = cmds.begin_render_pass({
                    .label           = std::nullopt,
                    .target          = swapchain.current_framebuffer(),
                    .begin_operation = siren::BeginOperation::Clear,
                    .clear_color     = siren::RGBA::black(),
            });

            pass.bind_graphics_pipeline(pipeline.handle());
            pass.bind_vertex_buffer(buffer.handle(), 0, 0);
            pass.draw_arrays(0, 3);

            cmds.consume_render_pass(pass.finish());
        }
        device->submit(cmds.finish());
        device->present(swapchain.handle());
        device->flush_delete_queue();
    }

    device->wait_until_idle();
    return 0;
}
