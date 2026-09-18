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
    f32 x, y;
    f32 r, g, b, a;
};

const auto vertices = ByteBuffer{
    Vertex{0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f},
    Vertex{-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f},
    Vertex{0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f},
};

auto main() -> i32 {
    auto       ctx       = Context::make({.level = log::Level::Trace});
    auto       window    = ctx.make_window({.title = "Example 01"});
    const auto device    = ctx.make_device();
    const auto swapchain = device->make_swapchain(window, {.vsync = true});

    AssetServer server{*device};

    const auto buffer = device->make_buffer(
        {
            .label = "Vertex Buffer",
            .size  = vertices.size_bytes(),
            .usage = BufferUsage::Static,
        },
        vertices.view()
    );
    const auto layout =
        LayoutBuilder::make().add(DataType::Float32, 2).add(DataType::Float32, 4).finish();

    const auto shaderh =
        server.load<ShaderAsset>("engine://examples/assets/shaders/load_shader.sshg");
    server.wait_until_loaded(shaderh);

    const auto pipeline = device->make_graphics_pipeline({
        .label             = "Load Shader Pipeline",
        .shader            = server.get<ShaderAsset>(shaderh)->shader.handle(),
        .layout            = layout,
        .color_attachments = GraphicsPipelineColorAttachments{
            GraphicsPipelineColorAttachment{
                .format      = swapchain.info().image_format,
                .alpha_mode  = AlphaMode::Opaque,
                .color_blend = {},
                .alpha_blend = {},
            },
        },
    });

    // main render loop
    while (!window.should_close()) {
        window.poll_events();

        auto backbuffer = swapchain.next_image();

        auto cmds = device->make_command_recorder();

        cmds.render_pass(
            RenderPassDescriptor{
                .label = "Load Shader Pass",
                .target =
                    RenderTarget{
                        .colors =
                            {
                                RenderPassColorAttachment{
                                    .image           = backbuffer,
                                    .clear_color     = Rgba::BLACK(),
                                    .begin_operation = BeginOperation::Clear,
                                    .end_operation   = EndOperation::Store,
                                },
                            },
                    },
            },
            [&](RenderCommandRecorder& pass) {
                pass.bind_graphics_pipeline(pipeline.handle());
                pass.bind_vertex_buffer(buffer.handle(), 0, 0);
                pass.draw_arrays(PrimitiveTopology::Triangles, 0, 3);
            }
        );

        swapchain.present(std::move(cmds).finish());
    }

    return 0;
}
