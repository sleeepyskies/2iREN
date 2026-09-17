#include "2iREN/asset/asset_server.hpp"
#include "2iREN/asset/shader.hpp"
#include "2iREN/core/context.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/layout.hpp"
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
    auto       ctx       = Context::make({.level = log::Level::Trace});
    auto       window    = ctx.make_window({.title = "Example 01"});
    const auto device    = ctx.make_device();
    const auto swapchain = device->make_swapchain(window, {.vsync = true});

    AssetServer server{*device};

    const auto buffer = device->make_buffer(
        {
            .label = "Sample Buffer",
            .size  = vertices.size_bytes(),
            .usage = BufferUsage::Static,
        },
        vertices.view()
    );
    const auto layout =
        LayoutBuilder::make().add(DataType::Float32, 3).add(DataType::Float32, 4).finish();

    const auto shaderh =
        server.load<ShaderAsset>("engine://examples/assets/shaders/load_shader.sshg");
    server.wait_until_loaded(shaderh);
    auto* shader_asset = server.get<ShaderAsset>(shaderh);

    const auto pipeline = device->make_graphics_pipeline({
        .label             = "Load Shader Pipeline",
        .shader            = shader_asset->shader.handle(),
        .layout            = layout,
        .color_attachments = GraphicsPipelineColorAttachments{
            GraphicsPipelineColorAttachment{
                .format      = ImageFormat::RGBA8,
                .alpha_mode  = AlphaMode::Opaque,
                .color_blend = BlendDescription{},
                .alpha_blend = BlendDescription{},
            },
        },
    });

    const RenderTarget target{
        .colors =
            {
                {
                    .image           = swapchain.next_image(),
                    .clear_color     = Rgba::BLACK(),
                    .begin_operation = BeginOperation::Clear,
                },
            },
        .depth_stencil = std::nullopt
    };

    // main render loop
    while (!window.should_close()) {
        window.poll_events();

        auto cmds = device->make_command_recorder();

        cmds.render_pass({.target = target}, [&](RenderCommandRecorder& pass) -> void {
            pass.bind_graphics_pipeline(pipeline.handle());
            pass.bind_vertex_buffer(buffer.handle(), 0, 0);
            pass.draw_arrays(PrimitiveTopology::Triangles, 0, 3);
        });

        device->present(swapchain.handle());
    }

    return 0;
}
