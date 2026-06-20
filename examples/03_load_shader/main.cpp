#include <unordered_map>

#include "2iren/2iren.hpp"
#include "2iren/asset/assets/gltf.hpp"
#include "2iren/asset/assets/shader.hpp"
#include "2iren/asset/asset_server.hpp"
#include "2iren/util/filesystem.hpp"

struct Vertex {
    siren::f32 x, y, z;
    siren::f32 r, g, b, a;
};

const siren::ByteBuffer vertices{
    Vertex{ .x = 0.0f, .y = 0.5f, .z = 0.0f, .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f },
    Vertex{ .x = -0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f },
    Vertex{ .x = 0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f },
};

int main() {
    siren::Context ctx{ { .debug = true, .level = siren::log::Level::Trace, .backend = siren::Backend::Auto } };
    siren::Window window({
        .title = "2iren",
        .width = 1280,
        .height = 800,
        .fullscreen = false,
        .vsync = true,
        .decorated = true,
        .resizable = true,
        .transparent = false,
    });
    auto device = ctx.create_device(window);
    siren::AssetServer server{*device};

    auto swapchain = device->create_swapchain({ .label = std::nullopt, .vsync = true });

    const auto buffer = device->create_buffer({
        .label = "sample_buffer",
        .data = vertices.data(),
        .size = vertices.size_bytes(),
        .usage = siren::BufferUsage::Static
    });
    const auto layout =
            siren::LayoutBuilder::start()
           .add(siren::Attribute::Position, 3, siren::DataType::Float32)
           .add(siren::Attribute::Color, 4, siren::DataType::Float32)
           .finish();

    const auto shader_handle = server.load<siren::ShaderAsset>("engine://examples/assets/shaders/basic.sshg");
    while (!server.is_loaded_with_dependencies(shader_handle)) { /* wait for load to finish */ }
    auto* shader_asset  = server.get<siren::ShaderAsset>(shader_handle);

    const auto pipeline = device->create_graphics_pipeline({
        .label = std::nullopt,
        .layout = layout,
        .shader = shader_asset->shader.handle(),
        .topology = siren::PrimitiveTopology::Triangles,
        .alpha_mode = siren::AlphaMode::Opaque,
        .depth_function = siren::DepthFunction::Less,
        .back_face_culling = true,
        .depth_test = true,
        .depth_write = true,
    });

    // main render loop
    while (!window.should_close()) {
        window.poll_events();

        auto cmds = device->record_render_commands();
        {
            auto pass = cmds.begin_render_pass({
                .label = std::nullopt,
                .target = swapchain.current_framebuffer(),
                .begin_operation = siren::BeginOperation::Clear,
                .clear_color = siren::RGBA::black(),
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
