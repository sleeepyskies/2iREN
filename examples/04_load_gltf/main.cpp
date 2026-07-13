#include "2iren/2iren.hpp"
#include "2iren/asset/asset_server.hpp"
#include "2iren/asset/assets/gltf.hpp"
#include "2iren/asset/assets/shader.hpp"
#include "2iren/util/camera.hpp"
#include "2iren/util/filesystem.hpp"

struct UBO {
    glm::mat4 view_proj;
};

auto render_node(
    siren::AssetServer& server, const siren::StrongHandle<siren::GltfNode>& nodeh, siren::RenderPassRecorder& pass)
    -> void {
    const auto& node = server.get_unsafe(nodeh);

    if (node.mesh) {
        const auto& mesh = server.get_unsafe(*node.mesh);

        for (const auto& surfaceh : mesh.surfaces) {
            const auto& surface = server.get_unsafe(surfaceh);
            pass.bind_vertex_buffer(surface.vertex_buffer.buffer.handle(), 0, 0);
            pass.bind_index_buffer(surface.index_buffer.buffer.handle(), surface.index_buffer.format);
            pass.draw_indexed(surface.index_buffer.count, 0);
        }
    }

    for (const auto& child : node.children) {
        render_node(server, child, pass);
    }
}

auto main() -> siren::i32 {
    siren::Context ctx{{.debug = true, .level = siren::log::Level::Trace, .backend = siren::Backend::Auto}};
    siren::Window window;
    auto device    = ctx.create_device(window);
    auto swapchain = device->create_swapchain({.label = std::nullopt, .vsync = true});
    siren::AssetServer server{*device};

    const auto shaderh = server.load<siren::ShaderAsset>("engine://examples/assets/shaders/load_gltf.sshg");
    const auto gltfh   = server.load<siren::Gltf>("engine://examples/assets/meshes/ABeautifulGame.glb");
    while (!server.is_loaded_with_dependencies(shaderh) || !server.is_loaded_with_dependencies(gltfh)) {
        /* wait for load to finish */
    }

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

    siren::PerspectiveCamera camera;
    const auto ubo = device->create_buffer({
        .label = "Camera UBO",
        .data  = std::nullopt,
        .size  = sizeof(UBO),
        .usage = siren::BufferUsage::Static,
    });

    // main render loop
    siren::log::info("Starting render loop.");
    while (!window.should_close()) {
        window.poll_events();

        const UBO data{.view_proj = camera.projection_view()};
        ubo.upload(siren::ByteBuffer{data});

        device->render_submit([&](siren::RenderCommandRecorder& cmds) -> void {
            cmds.render_pass(
                {.clear_color = siren::RGBA::red(),
                    .target   = siren::RenderTarget{.colors = {swapchain.next_image()}, .depth_stencil = std::nullopt}},
                [&](siren::RenderPassRecorder& pass) -> void {
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

    device->wait_until_idle();
    return 0;
}
