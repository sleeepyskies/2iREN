#include "2iREN/asset/asset_server.hpp"
#include "2iREN/asset/shader.hpp"
#include "2iREN/core/context.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/layout.hpp"
#include "2iREN/graphics/render_command.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/window/window.hpp"

using namespace siren;

struct Vertex {
    f32 x, y, z;
    f32 r, g, b, a;
};

const ByteBuffer vertices{
    Vertex{.x = 0.0f, .y = 0.5f, .z = 0.0f, .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = -0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f},
    Vertex{.x = 0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f},
};

int main() {
    const auto ctx    = Context::make({
        .debug = true,
        .level = log::Level::Trace,
    });
    auto window       = ctx.make_window({.title = "Example 03"});
    const auto device = ctx.make_device();
    AssetServer server{*device};

    const auto swapchain = device->make_swapchain(
        window,
        {
            .label = std::nullopt,
            .vsync = true,
        }
    );

    const auto buffer = device->make_buffer(
        {
            .label = "Sample Buffer",
            .size  = vertices.size_bytes(),
            .usage = BufferUsage::Static,
        },
        vertices.view()
    );
    const auto layout = LayoutBuilder::create()
                            .add(Attribute::Position, 3, DataType::Float32)
                            .add(Attribute::Color, 4, DataType::Float32)
                            .finish();

    const auto shaderh =
        server.load<ShaderAsset>("engine://examples/assets/shaders/load_shader.sshg");
    server.wait_until_loaded(shaderh);
    auto* shader_asset = server.get<ShaderAsset>(shaderh);

    const auto pipeline = device->make_graphics_pipeline({
        .label             = std::nullopt,
        .layout            = layout,
        .shader            = shader_asset->shader.handle(),
        .topology          = PrimitiveTopology::Triangles,
        .alpha_mode        = AlphaMode::Opaque,
        .depth_function    = DepthFunction::Less,
        .back_face_culling = true,
        .depth_test        = true,
        .depth_write       = true,
    });

    const RenderTarget target{
        .colors =
            {
                {
                    .image           = swapchain.next_image(),
                    .begin_operation = BeginOperation::Clear,
                    .clear_color     = Rgba::BLACK(),
                },
            },
        .depth_stencil = std::nullopt
    };

    // main render loop
    while (!window.should_close()) {
        window.poll_events();

        device->render_pass({.target = target}, [&](RenderPassRecorder& pass) -> void {
            pass.bind_graphics_pipeline(pipeline.handle());
            pass.bind_vertex_buffer(buffer.handle(), 0, 0);
            pass.draw_fullscreen();
        });

        device->present(swapchain.handle());
    }

    return 0;
}
