#include "2iREN/container/byte_buffer.hpp"
#include "2iREN/core/context.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/commands.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/layout.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/graphics/types.hpp"
#include "2iREN/math/angle.hpp"
#include "2iREN/math/mat4x4.hpp"
#include "2iREN/window/window.hpp"

using namespace siren;

struct Vertex {
    Vec3f pos;
};

struct UboData {
    Mat4x4f transform;
};

#ifdef SIREN_MACOS
const auto shader_source = R"(
#include <metal_stdlib>
using namespace metal;

struct VertexOut {
    float4 position [[position]];
    float3 local_pos;
};

struct VertexIn {
    float3 position [[attribute(0)]];
};

struct Uniforms {
    float4x4 transform;
};

auto vertex vmain(
    VertexIn in [[stage_in]],
    constant Uniforms& uniforms [[buffer(1)]]
) -> VertexOut {
    return VertexOut {
        uniforms.transform * float4(in.position, 1.0),
        in.position,
    };
}

auto fragment fmain(VertexOut in [[stage_in]]) -> float4 {
    return float4(in.local_pos.xyz + 0.5, 1.0);
}
)";
const auto vertex_shader = ShaderData{
    .label  = std::nullopt,
    .source = shader_source,
    .entry  = "vmain",
};
const auto fragment_shader = ShaderData{
    .label  = std::nullopt,
    .source = shader_source,
    .entry  = "fmain",
};
#else
const auto vertex_shader = ShaderData{
    .label  = std::nullopt,
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
const auto fragment_shader = ShaderData{
    .label  = std::nullopt,
    .source = R"(
        #version 460
        layout(location = 0) in vec3 v_pos;

        layout(location = 0) out vec4 FragColor;

        void main() {
            FragColor = vec4(v_pos + 0.5, 1.0);
        })",
};
#endif

const std::unordered_map<ShaderStage, ShaderData> shaders = {
    {ShaderStage::Vertex, vertex_shader},
    {ShaderStage::Fragment, fragment_shader},
};

const auto vertices = ByteBuffer{
    Vertex{Vec3f{-0.5f, -0.5f, -0.5f}},
    Vertex{Vec3f{0.5f, -0.5f, -0.5f}},
    Vertex{Vec3f{0.5f, 0.5f, -0.5f}},
    Vertex{Vec3f{-0.5f, 0.5f, -0.5f}},
    Vertex{Vec3f{-0.5f, -0.5f, 0.5f}},
    Vertex{Vec3f{0.5f, -0.5f, 0.5f}},
    Vertex{Vec3f{0.5f, 0.5f, 0.5f}},
    Vertex{Vec3f{-0.5f, 0.5f, 0.5f}},
};
const auto indices = ByteBuffer::make<u32>({
    // clang-format off
    0, 1, 2, 2, 3, 0, // front
    1, 5, 6, 6, 2, 1, // right
    7, 6, 5, 5, 4, 7, // back
    4, 0, 3, 3, 7, 4, // left
    4, 5, 1, 1, 0, 4, // bottom
    3, 2, 6, 6, 7, 3  // top
    // clang-format on
});

auto main() -> i32 {
    auto ctx             = Context::make({.level = log::Level::Trace});
    auto window          = ctx.make_window({.title = "Example 02"});
    const auto device    = ctx.make_device();
    const auto swapchain = device->make_swapchain(
        window,
        {
            .extent = window.framebuffer_extent(),
            .vsync  = true,
        }
    );

    const auto vertex_buffer = device->make_buffer(
        {
            .label        = "Cube Vertices",
            .size         = vertices.size_bytes(),
            .usage        = BufferFlags::make(BufferFlag::Vertex),
            .memory_usage = MemoryUsage::CpuAndGpu,
        },
        vertices.view()
    );
    const auto index_buffer = device->make_buffer(
        {
            .label        = "Cube Indicies",
            .size         = indices.size_bytes(),
            .usage        = BufferFlags::make(BufferFlag::Index),
            .memory_usage = MemoryUsage::CpuAndGpu,
        },
        indices.view()
    );
    const auto uniform_buffer = device->make_buffer({
        .label        = "Uniform Buffer",
        .size         = sizeof(UboData),
        .usage        = BufferFlags::make(BufferFlag::Uniform),
        .memory_usage = MemoryUsage::CpuAndGpu,
    });
    const auto layout         = LayoutBuilder::make().add(DataType::Float32, 3).finish();

    const auto shader = device->make_shader({.label = "Cube Shader", .source = shaders});

    const auto pipeline = device->make_graphics_pipeline({
        .label  = "Cube Pipeline",
        .shader = shader.handle(),
        .layout = layout,
        .colors =
            ColorAttachmentDescriptors{
                ColorAttachmentDescriptor{
                    .format     = swapchain.info().image_format,
                    .alpha_mode = AlphaMode::Opaque,
                },
            },
        .depth_stencil = DepthStencilAttachmentDescriptor{.format = ImageFormat::Depth32f},
    });

    const auto quarter_angle = Degrees{45}.to_radians();
    u32 count                = 0;

    while (!window.should_close()) {
        window.poll_events();

        const auto model = Mat4x4f::rotate(
            Mat4x4f::IDENTITY(),
            Degrees{count * 0.1f}.to_radians(),
            Vec3f{0.5f, 1.0f, 0.0f}
        );
        const auto view = Mat4x4f::translate(Mat4x4f::IDENTITY(), Vec3f{0.0f, 0.0f, 5.0f});
        const auto proj = Mat4x4f::perspective(quarter_angle, window.aspect(), 0.1f, 10.f);
        const auto data = ByteBuffer{UboData{proj * view * model}};

        auto backbuffer = swapchain.next_image();

        auto cmds = device->make_command_buffer();

        cmds->write_buffer(uniform_buffer.handle(), 0, data.view());

        cmds->render_pass(
            RenderPassDescriptor{
                .label = "Cube Pass",
                .target =
                    RenderTarget{
                        .colors =
                            TargetColorAttachments{
                                TargetColorAttachment{
                                    .image           = backbuffer,
                                    .clear_color     = siren::Rgba::BLACK(),
                                    .begin_operation = BeginOperation::Clear,
                                    .end_operation   = EndOperation::Store,
                                },
                            },
                        .depth_stencil = std::nullopt,
                    },
            },
            [&](RenderCommandEncoder& pass) -> void {
                pass.bind_graphics_pipeline(pipeline.handle());

                pass.bind_vertex_buffer(vertex_buffer.handle(), Slot{0});
                pass.bind_uniform_buffer(uniform_buffer.handle(), Slot{1});
                pass.bind_index_buffer(index_buffer.handle(), IndexType::UInt32);

                pass.draw_indexed(indices.size_as<u32>());
            }
        );

        swapchain.present(std::move(cmds));

        count++;
    }

    return 0;
}
