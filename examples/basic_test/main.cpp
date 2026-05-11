#include <GLFW/glfw3.h>

#include "2iren/2iren.hpp"
#include "2iren/resources/shader.hpp"
#include "2iren/util/buf.hpp"

/// @todo: window abstraction + swapchain + surface or something

struct Vertex {
    siren::f32 x, y, z;
    siren::f32 r, g, b, a;
};

const siren::ShaderStageData vertex_shader{
    .label = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec4 aColor;

        void main() {
            gl_Position = vec4(aPos, 1.0);
        })",
};

const siren::ShaderStageData fragment_shader{
    .label = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) out vec4 FragColor;

        void main() {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        })"
};
const std::unordered_map<siren::ShaderStage, siren::ShaderStageData> shaders = {
    { siren::ShaderStage::Vertex,  vertex_shader },
    { siren::ShaderStage::Fragment, fragment_shader },
};
const siren::ByteBuffer vertices {
    Vertex{ .x = 0.0f, .y = 0.5f,  .z = 0.0f,  .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 1.0f },
    Vertex{ .x = -0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 1.0f, .b = 0.0f, .a = 1.0f },
    Vertex{ .x = 0.5f, .y = -0.5f, .z = 0.0f, .r = 0.0f, .g = 0.0f, .b = 1.0f, .a = 1.0f },
};

int main() {
    siren::init({
        .level = siren::log::Level::Trace
    });

    if (!glfwInit()) {
        siren::log::error("Could not init glfw");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = glfwCreateWindow(1280, 800, "hi", nullptr, nullptr);
    if (!window) {
        siren::log::error("Could not create a window.");
        return -1;
    }
    glfwMakeContextCurrent(window);

    const auto device = siren::Device::create({
        .backend = siren::Backend::Auto,
        .window = window,
        .debug = true,
    });
    const auto buffer           = device->create_buffer({
        .label = "sample_buffer",
        .data = vertices.data(),
        .size = vertices.size_bytes(),
        .usage = siren::BufferUsage::Static,
    });
    const auto layout =
            siren::VertexLayoutBuilder::start()
           .add(siren::Component::Position, 3, siren::DataType::Float32)
           .add(siren::Component::Color, 4, siren::DataType::Float32)
           .finish();

    const auto target = device->create_framebuffer({
        .label = std::nullopt,
        .width = 1280,
        .height = 800,
        .num_colors = 1,
        .has_depth_stencil = false,
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

    while (!glfwWindowShouldClose(window)) {
        auto cmds = device->record_render_commands();
        {
            auto pass = cmds.begin_render_pass({
                .label = std::nullopt,
                .target = target.handle(),
                .begin_operation = siren::BeginOperation::Clear,
                .clear_color = siren::RGBA::black(),
            });

            pass.bind_graphics_pipeline(pipeline.handle());
            pass.bind_vertex_buffer(buffer.handle(), 0, 0);
            pass.draw_arrays(0, 3);

            cmds.consume_render_pass(pass.finish());
        }
        device->submit(cmds.finish());
        device->flush_delete_queue();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
