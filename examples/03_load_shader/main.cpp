#include "2iren/2iren.hpp"
#include "2iren/asset/asset_server.hpp"
#include "2iren/asset/assets/gltf.hpp"
#include "2iren/asset/assets/shader.hpp"
#include "2iren/util/filesystem.hpp"

struct Vertex {
    siren::f32 x, y, z;
    siren::f32 r, g, b, a;
};

const siren::ByteBuffer vertices{
    Vertex{.x = 0.0f, .y = 0.5f, .z = 0.0f, .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = -0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = 0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f},
};

int main() {
    const auto ctx = siren::Context::create(
        {
            .debug   = true,
            .level   = siren::log::Level::Trace,
            .backend = siren::Backend::Auto
        }
    );
    auto window       = ctx.create_window({});
    const auto device = ctx.create_device({.window = window});
    siren::AssetServer server{*device};

    const auto swapchain = device->create_swapchain(
        {
            .label  = std::nullopt,
            .vsync  = true,
            .extent = window.size(),
            .window = &window
        }
    );

    const auto buffer = device->create_buffer(
        {
            .label = "Sample Buffer",
            .data  = vertices.data(),
            .size  = vertices.size_bytes(),
            .usage = siren::BufferUsage::Static
        }
    );
    const auto layout = siren::LayoutBuilder::create()
                        .add(siren::Attribute::Position, 3, siren::DataType::Float32)
                        .add(siren::Attribute::Color, 4, siren::DataType::Float32)
                        .finish();

    const auto shaderh = server.load<siren::ShaderAsset>("engine://examples/assets/shaders/load_shader.sshg");
    server.wait_until_loaded(shaderh);
    auto* shader_asset = server.get<siren::ShaderAsset>(shaderh);

    const auto pipeline = device->create_graphics_pipeline(
        {
            .label             = std::nullopt,
            .layout            = layout,
            .shader            = shader_asset->shader.handle(),
            .topology          = siren::PrimitiveTopology::Triangles,
            .alpha_mode        = siren::AlphaMode::Opaque,
            .depth_function    = siren::DepthFunction::Less,
            .back_face_culling = true,
            .depth_test        = true,
            .depth_write       = true,
        }
    );

    const siren::RenderTarget target{
        .colors = {
            {
                .image           = swapchain.next_image(),
                .begin_operation = siren::BeginOperation::Clear,
                .clear_color     = siren::Rgba::black()
            },
        },
        .depth_stencil = std::nullopt
    };

    // main render loop
    while (!window.should_close()) {
        window.poll_events();

        device->render_submit(
            [&](siren::RenderCommandRecorder& cmds) -> void {
                cmds.render_pass(
                    {.target = target},
                    [&](siren::RenderPassRecorder& pass) -> void {
                        pass.bind_graphics_pipeline(pipeline.handle());
                        pass.bind_vertex_buffer(buffer.handle(), 0, 0);
                        pass.draw_fullscreen();
                    }
                );
            }
        );

        device->present(swapchain.handle());
        device->flush_delete_queue();
    }

    device->wait_until_idle();
    return 0;
}
