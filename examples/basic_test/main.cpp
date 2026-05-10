#include <GLFW/glfw3.h>

#include "2iren/2iren.hpp"

int main() {
    siren::log::init();

    glfwInit();
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

    const auto device = siren::Device::create(
        {
            .backend = siren::Backend::Auto,
            .window = window,
        }
    );
    std::vector<siren::u8> data = { 1, 2, 4, 5, 6, 7 };
    const auto buffer           = device->create_buffer(
        {
            .label = "sample_buffer",
            .data = data,
            .size = data.size(),
            .usage = siren::BufferUsage::Static,
        }
    );

    while (!glfwWindowShouldClose(window)) {
        auto cmds = device->record_render_commands();
        {
            auto pass = cmds.begin_render_pass(
                {
                    .label = std::nullopt,
                    .target = siren::NullHandle,
                    .begin_operation = siren::BeginOperation::Clear,
                    .clear_color = siren::RGBA{ 1, 2, 4, 1 },
                }
            );

            pass.bind_vertex_buffer(buffer.handle(), 0, 0);

            cmds.consume_render_pass(pass.finish());
        }
        device->submit(cmds.finish());
        device->flush_delete_queue();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
