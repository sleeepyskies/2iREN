#include "2iren/2iren.hpp"

int main() {
    const auto device = siren::Device::create(
        {
            .backend = siren::Backend::Auto,
            .window = nullptr,
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

    while (true) {
        auto cmds = device->record_render_commands();
        {
            auto pass = cmds.begin_render_pass(
                {
                    .label = "nothing",
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
    return 0;
}
