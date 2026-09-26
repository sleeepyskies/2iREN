#include "commands.hpp"
#include "2iREN/graphics/commands.hpp"

namespace siren {

auto RenderCommandEncoder::bind_graphics_pipeline(GraphicsPipelineHandle pipeline) -> void { }

auto CommandBuffer::render_pass(const RenderPassDescriptor& descriptor, RenderPassFunction&& encode)
    -> void {
    auto command_encoder = RenderCommandEncoder{};
    std::invoke(encode, command_encoder);
}

} // namespace siren
