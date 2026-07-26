#include "context.hpp"

#include "device.hpp"
#include "2iren/util/filesystem.hpp"
#include "backend/gl/device.hpp"
#include "2iren/util/platform.hpp"
#include "2iren/util/time.hpp"
#include <stb_image.h>

#include "2iren/sync/thread_pool.hpp"
#include "2iren/window.hpp"

#ifndef SIREN_ENGINE_ROOT
#define SIREN_ENGINE_ROOT "."
#endif

namespace siren {

static auto select_gl_backend() -> void {
    log::info("OpenGL backend chosen.");
    stbi_set_flip_vertically_on_load(true); // true only for
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

static auto create_gl_device(const Window& window) -> std::unique_ptr<Device> {
    log::info("Creating an OpenGlDevice.");
    return std::make_unique<GlDevice>(window.handle());
}

Context::Context(const ContextDescriptor& descriptor) : m_descriptor(descriptor) {
    log::init(descriptor.level);
    libassert::set_failure_handler(
        [] (const libassert::assertion_info& info){
            log::error("{}", info);
            std::abort();
        }
    );
    glfwSetErrorCallback(
        [] (i32 err, const char* desc){
            log::error("GLFW Error encountered. Code: {}, description: {}", err, desc);
        }
    );

    ASSERT(glfwInit(), "Could not initialize GLFW.");

    time::init();

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
    if constexpr (!SINGLE_THREADED) {
        ThreadPool::init();
    }

    // init asset server

    // init virtual filesystem
    const auto engine_root = Path{ SIREN_ENGINE_ROOT };
    FileSystem::mount("engine", engine_root);
}

auto Context::create(const ContextDescriptor& descriptor) -> Context {
    static bool called = false;
    if (called) {
        throw std::runtime_error("Context already created.");
    }
    called = true;
    return Context{ descriptor };
}

Context::~Context() {
    if constexpr (!SINGLE_THREADED) {
        ThreadPool::shutdown();
    }
}

auto Context::create_device(const Window& window) const -> std::unique_ptr<Device> {
    switch (m_descriptor.backend) {
        case Backend::OpenGL: {
            return create_gl_device(window);
        }
        default: UNREACHABLE("Invalid Backend.");
    }
}

auto Context::create_window(const WindowDescriptor& descriptor) const -> Window {
    return Window{ descriptor };
}

} // namespace siren
