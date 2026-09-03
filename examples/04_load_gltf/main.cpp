#include "2iREN/asset/asset_server.hpp"
#include "2iREN/asset/gltf.hpp"
#include "2iREN/asset/shader.hpp"
#include "2iREN/context.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/render_target.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/math/mat4x4.hpp"
#include "2iREN/scene/camera.hpp"
#include "2iREN/window.hpp"

using namespace siren;

struct UBO {
    siren::Mat4x4f view_proj;
};

auto render_node(
    siren::AssetServer& server,
    const siren::StrongHandle<siren::GltfNode>& nodeh,
    siren::RenderPassRecorder& pass
) -> void {
    const auto& node = server.get_unsafe(nodeh);

    if (node.mesh) {
        const auto& mesh = server.get_unsafe(*node.mesh);

        for (const auto& surfaceh : mesh.surfaces) {
            const auto& surface = server.get_unsafe(surfaceh);
            pass.bind_vertex_buffer(surface.vertex_buffer.buffer.handle(), 0, 0);
            pass.bind_index_buffer(
                surface.index_buffer.buffer.handle(), surface.index_buffer.format
            );
            pass.draw_indexed(surface.index_buffer.count, 0);
        }
    }

    for (const auto& child : node.children) {
        render_node(server, child, pass);
    }
}

auto main() -> siren::i32 {
    const auto ctx = siren::Context::create(
        {.debug = true, .level = siren::log::Level::Trace, .backend = siren::Backend::Auto}
    );
    auto window          = ctx.create_window({});
    const auto device    = ctx.create_device({.window = window});
    const auto swapchain = device->create_swapchain(
        {.label = std::nullopt, .vsync = true, .extent = window.extent(), .window = &window}
    );
    siren::AssetServer server{*device};

    const auto shaderh =
        server.load<siren::ShaderAsset>("engine://examples/assets/shaders/load_gltf.sshg");
    const auto gltfh =
        server.load<siren::Gltf>("engine://examples/assets/meshes/ABeautifulGame.glb");
    server.wait_until_loaded(shaderh);
    server.wait_until_loaded(gltfh);

    const auto pipeline = device->create_graphics_pipeline({
        .label             = std::nullopt,
        .layout            = siren::DEFAULT_VERTEX_LAYOUT,
        .shader            = server.get<siren::ShaderAsset>(shaderh)->shader.handle(),
        .topology          = siren::PrimitiveTopology::Triangles,
        .alpha_mode        = siren::AlphaMode::Opaque,
        .depth_function    = siren::DepthFunction::Less,
        .back_face_culling = true,
        .depth_test        = true,
        .depth_write       = true,
    });

    const auto& gltf  = server.get_unsafe(gltfh);
    const auto& scene = server.get_unsafe(*gltf.default_scene);

    auto camera    = siren::Camera({});
    const auto ubo = device->create_buffer({
        .label = "Camera UBO",
        .data  = std::nullopt,
        .size  = sizeof(UBO),
        .usage = siren::BufferUsage::Static,
    });

    const siren::RenderTarget target{
        .colors =
            {
                {
                    .image           = swapchain.next_image(),
                    .begin_operation = siren::BeginOperation::Clear,
                    .clear_color     = siren::Rgba::BLACK(),
                },
            },
        .depth_stencil = std::nullopt
    };

    // main render loop
    while (!window.should_close()) {
        window.poll_events();

        const UBO data{
            .view_proj = camera.projection_view((f32)window.width() / (f32)window.height())
        };
        ubo.upload(siren::ByteBuffer{data});

        device->render_submit([&](siren::RenderCommandRecorder& cmds) -> void {
            cmds.render_pass({.target = target}, [&](siren::RenderPassRecorder& pass) -> void {
                pass.bind_graphics_pipeline(pipeline.handle());
                pass.bind_uniform_buffer(ubo.handle(), 0);

                for (const auto& node : scene.root_nodes) {
                    render_node(server, node, pass);
                }
            });
        });

        swapchain.present();
        device->flush_delete_queue();
    }

    return 0;
}
