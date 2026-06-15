#include <unordered_map>

#include "2iren/2iren.hpp"
#include "2iren/util/byte_buffer.hpp"

const siren::ShaderStageData vertex_shader{
    .label = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) in vec3 a_pos;
        layout(location = 0) out vec3 v_pos;

        layout(binding = 0) uniform UBO {
            mat4 rot;
        };

        void main() {
            gl_Position = rot * vec4(a_pos, 1.0);
            v_pos = a_pos;
        })",
};
const siren::ShaderStageData fragment_shader{
    .label = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) in vec3 v_pos;

        layout(location = 0) out vec4 FragColor;

        void main() {
            FragColor = vec4(v_pos + 0.5, 1.0);
        })"
};

const std::unordered_map<siren::ShaderStage, siren::ShaderStageData> shaders = {
    { siren::ShaderStage::Vertex,  vertex_shader },
    { siren::ShaderStage::Fragment, fragment_shader },
};

int main() {
    // init siren
    siren::Context ctx{{
        .debug = true,
        .level = siren::log::Level::Trace,
        .backend = siren::Backend::Auto,
    }};

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
    auto swapchain = device->create_swapchain({
        .label = std::nullopt,
        .vsync = true,
    });

    const auto shader = device->create_shader({
        .label = std::nullopt,
        .source = shaders,
    });

    const auto pipeline = device->create_graphics_pipeline({
        .label = std::nullopt,
        .layout = layout,
        .shader = shader.handle(),
        .topology = siren::PrimitiveTopology::Triangles,
        .alpha_mode = siren::AlphaMode::Opaque,
        .depth_function = siren::DepthFunction::Less,
        .back_face_culling = true,
        .depth_test = true,
        .depth_write = true,
    });

    ctx.assets().register_asset<asset::Mesh>();

    while (!window.should_close()) {
        window.poll_events();

        device->submit(render_cmds.finish());
        device->present(swapchain.handle());
        device->flush_delete_queue();
        cnt++;
    }

    device->wait_until_idle();
    return 0;
}
