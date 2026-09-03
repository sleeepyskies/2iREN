#include "2iREN/context.hpp"
#include "2iREN/graphics/buffer.hpp"
#include "2iREN/graphics/graphics_pipeline.hpp"
#include "2iREN/graphics/layout.hpp"
#include "2iREN/graphics/shader.hpp"
#include "2iREN/graphics/swapchain.hpp"
#include "2iREN/math/angle.hpp"
#include "2iREN/math/extent.hpp"
#include "2iREN/math/mat4x4.hpp"
#include "2iREN/utility/byte_buffer.hpp"
#include "2iREN/window.hpp"

using namespace siren;

struct Vertex {
    f32 x, y, z;
};

struct UboData {
    Mat4x4f transform;
};

const ShaderData vertex_shader{
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
const ShaderData fragment_shader{.label = std::nullopt, .source = R"(
        #version 460
        layout(location = 0) in vec3 v_pos;

        layout(location = 0) out vec4 FragColor;

        void main() {
            FragColor = vec4(v_pos + 0.5, 1.0);
        })"};

const std::unordered_map<ShaderStage, ShaderData> shaders = {
    {ShaderStage::Vertex, vertex_shader},
    {ShaderStage::Fragment, fragment_shader},
};
const ByteBuffer vertices{
    Vertex{-0.5f, -0.5f, 0.5f},
    Vertex{0.5f, -0.5f, 0.5f},
    Vertex{0.5f, 0.5f, 0.5f},
    Vertex{-0.5f, 0.5f, 0.5f},
    Vertex{-0.5f, -0.5f, -0.5f},
    Vertex{0.5f, -0.5f, -0.5f},
    Vertex{0.5f, 0.5f, -0.5f},
    Vertex{-0.5f, 0.5f, -0.5f},
};
const ByteBuffer indices = [] {
    ByteBuffer buf;
    // clang-format off
    buf.append<u32>({
        0, 1, 2, 2, 3, 0, // front
        1, 5, 6, 6, 2, 1, // right
        7, 6, 5, 5, 4, 7, // back
        4, 0, 3, 3, 7, 4, // left
        4, 5, 1, 1, 0, 4, // bottom
        3, 2, 6, 6, 7, 3  // top
    });
    // clang-format on
    return buf;
}();

int main() {
    // init siren
    const auto ctx =
        Context::create({.debug = true, .level = log::Level::Trace, .backend = Backend::Auto});
    auto window = ctx.create_window({});

    const auto device    = ctx.create_device({.window = window});
    const auto swapchain = device->create_swapchain({
        .label  = std::nullopt,
        .vsync  = true,
        .extent = window.extent(),
        .window = &window,
    });

    const auto vertex_buffer  = device->create_buffer({
        .label = "cube_buffer",
        .data  = vertices.data(),
        .size  = vertices.size_bytes(),
        .usage = BufferUsage::Static,
    });
    const auto index_buffer   = device->create_buffer({
        .label = "cube_indices",
        .data  = indices.data(),
        .size  = indices.size_bytes(),
        .usage = BufferUsage::Static,
    });
    const auto uniform_buffer = device->create_buffer({
        .label = "uniform_buffer",
        .data  = std::nullopt,
        .size  = sizeof(UboData),
        .usage = BufferUsage::Dynamic,
    });
    const auto layout =
        LayoutBuilder::create().add(Attribute::Position, 3, DataType::Float32).finish();

    const auto shader   = device->create_shader({
        .label  = std::nullopt,
        .source = shaders,
    });
    const auto pipeline = device->create_graphics_pipeline({
        .label             = std::nullopt,
        .layout            = layout,
        .shader            = shader.handle(),
        .topology          = PrimitiveTopology::Triangles,
        .alpha_mode        = AlphaMode::Opaque,
        .depth_function    = DepthFunction::Less,
        .back_face_culling = true,
        .depth_test        = true,
        .depth_write       = true,
    });

    const auto color = device->create_image({
        .format        = ImageFormat::RGBA8,
        .extent        = window.extent().to_extent3(),
        .dimension     = ImageDimension::D2,
        .mipmap_levels = 1,
    });
    const RenderTarget target{
        .colors =
            {
                {
                    .image           = color.handle(),
                    .begin_operation = BeginOperation::Clear,
                    .clear_color     = Rgba::BLACK(),
                },
            },
        .depth_stencil = std::nullopt
    };

    u32 count = 0;
    log::info("Starting main loop");
    while (!window.should_close()) {
        window.poll_events();

        const auto model = Mat4x4f::rotate(
            Mat4x4f::IDENTITY(), Degrees{count * 0.1f}.to_radians(), Vec3f{0.5f, 1.0f, 0.0f}
        );
        const auto view = Mat4x4f::translate(Mat4x4f::IDENTITY(), Vec3f{0.0f, 0.0f, -2.0f});
        const auto proj = Mat4x4f::perspective(
            Degrees{45}.to_radians(), (f32)window.width() / (f32)window.height(), 0.1f, 10.f
        );
        const UboData ubodata{proj * view * model};
        ByteBuffer ubo{ubodata};

        device->resource_submit([&](ResourceCommandRecorder& cmds) -> void {
            cmds.upload_to_buffer(uniform_buffer.handle(), ubo, 0);
        });

        device->render_pass({.target = target}, [&](RenderPassRecorder& pass) -> void {
            pass.bind_graphics_pipeline(pipeline.handle());
            pass.bind_vertex_buffer(vertex_buffer.handle(), 0, 0);
            pass.bind_index_buffer(index_buffer.handle(), IndexFormat::UInt32);
            pass.bind_uniform_buffer(uniform_buffer.handle(), 0);
            pass.draw_indexed(indices.size_as<u32>(), 0);
        });

        device->blit_image(target.colors[0].image, swapchain.next_image());

        device->present(swapchain.handle());
        device->flush_delete_queue();
        count++;
    }

    return 0;
}
