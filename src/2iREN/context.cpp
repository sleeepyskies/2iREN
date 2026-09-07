#include "context.hpp"

#include <stb/stb_image.h>

#include "2iREN/concurrency/thread_pool.hpp"
#include "2iREN/graphics/backend/opengl/device.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/utility/filesystem.hpp"
#include "2iREN/utility/platform.hpp"
#include "2iREN/utility/time.hpp"
#include "2iREN/window.hpp"

#ifndef SIREN_ENGINE_ROOT
#define SIREN_ENGINE_ROOT "."
#endif

namespace siren {
static auto select_gl_backend() -> void {
    log::info("opengl backend chosen.");
    stbi_set_flip_vertically_on_load(true); // true only for
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

static auto create_gl_device() -> std::unique_ptr<Device> {
    log::info("creating an OpenGL device.");
    return std::make_unique<GlDevice>();
}

Context::Context(const ContextDescriptor& descriptor) : m_descriptor(descriptor) {
    log::init(descriptor.level);
    glfwSetErrorCallback([](i32 err, const char* desc) {
        PANIC("GLFW error encountered. code: {}, description: {}", err, desc);
    });

    ASSERT(glfwInit(), "could not initialize GLFW.");

    time::initialize();

    // select backend
    switch (descriptor.backend) {
        case Backend::Auto: {
            log::info("Autoselecting a backend.");
            if constexpr (platform::current == platform::Windows) {
                m_descriptor.backend = Backend::OpenGL;
                select_gl_backend();
                break;
            } else if constexpr (platform::current == platform::Linux) {
                m_descriptor.backend = Backend::OpenGL;
                select_gl_backend();
                break;
            } else {
                PANIC("Unsupported platform");
            }
        }
        case Backend::OpenGL: {
            select_gl_backend();
            break;
        }
    }

    // init async stuffs
    ThreadPool::init();

    // init asset server

    // init virtual filesystem
    const auto engine_root = Path{SIREN_ENGINE_ROOT};
    FileSystem::mount("engine", engine_root);
}

auto Context::create(const ContextDescriptor& descriptor) -> Context {
    static bool called = false;
    if (called) {
        throw std::runtime_error("Context already created.");
    }
    called = true;
    return Context{std::move(descriptor)};
}

Context::~Context() {
    if constexpr (!SINGLE_THREADED) {
        ThreadPool::shutdown();
    }
}

auto Context::create_device() const -> std::unique_ptr<Device> {
    switch (m_descriptor.backend) {
        case Backend::OpenGL: {
            return create_gl_device();
        }
        default: UNREACHABLE();
    }
}

auto Context::create_window(const WindowDescriptor& descriptor) const -> Window {
    return Window{descriptor};
}
} // namespace siren
