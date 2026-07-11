#include "2iren/2iren.hpp"
#include "2iren/asset/asset_server.hpp"
#include "2iren/asset/assets/gltf.hpp"
#include "2iren/asset/assets/shader.hpp"
#include "2iren/util/filesystem.hpp"

int main() {
    siren::Context ctx{{.debug = true, .level = siren::log::Level::Trace, .backend = siren::Backend::Auto}};
    siren::Window window;
    auto device    = ctx.create_device(window);
    auto swapchain = device->create_swapchain({.label = std::nullopt, .vsync = true});
    siren::AssetServer server{*device};

    const auto shader_handle = server.load<siren::ShaderAsset>("engine://examples/assets/shaders/default.sshg");
    const auto gltf_handle   = server.load<siren::Gltf>("engine://examples/assets/meshes/ABeautifulGame.glb");
    while (!server.is_loaded_with_dependencies(shader_handle) ||
        !server.is_loaded_with_dependencies(gltf_handle)) { /* wait for load to finish */
    }

    auto* shader_asset = server.get<siren::ShaderAsset>(shader_handle);

    // model has just one mesh, with just one surface
    auto* gltf    = server.get<siren::Gltf>(gltf_handle);
    auto* mesh    = server.get<siren::Mesh>(gltf->meshes[0]);
    auto* surface = server.get<siren::Surface>(mesh->surfaces[0]);

    const auto pipeline = device->create_graphics_pipeline({
        .label             = std::nullopt,
        .layout            = surface->vertex_buffer.layout,
        .shader            = shader_asset->shader.handle(),
        .topology          = siren::PrimitiveTopology::Triangles,
        .alpha_mode        = siren::AlphaMode::Opaque,
        .depth_function    = siren::DepthFunction::Less,
        .back_face_culling = true,
        .depth_test        = true,
        .depth_write       = true,
    });

    // main render loop
    siren::log::info("Starting render loop.");
    while (!window.should_close()) {
        window.poll_events();

        device->render_submit([&](siren::RenderCommandRecorder& cmds) -> void {
            cmds.render_pass({.target = siren::RenderTarget{swapchain.next_image().handle()}},
                [&](siren::RenderPassRecorder& pass) -> void {
                    pass.bind_graphics_pipeline(pipeline.handle());
                    pass.bind_vertex_buffer(surface->vertex_buffer.buffer.handle(), 0, 0);
                    pass.bind_index_buffer(surface->index_buffer.buffer.handle(), surface->index_buffer.format);
                    pass.draw_indexed(surface->index_buffer.count, 0);
                });
        });

        device->present(swapchain.handle());
        device->flush_delete_queue();
    }

    device->wait_until_idle();
    return 0;
}
