#include "context.hpp"

#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include "2iREN/concurrency/thread_pool.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/utility/filesystem.hpp"
#include "2iREN/utility/time.hpp"
#include "2iREN/window.hpp"

#if defined(SIREN_LINUX) || defined(SIREN_WINDOWS)
#include "2iREN/graphics/backend/opengl/device.hpp"
#elifdef SIREN_MACOS
#include "2iREN/graphics/backend/metal/device.hpp"
#endif

#ifndef SIREN_ENGINE_ROOT
#define SIREN_ENGINE_ROOT "."
#endif

namespace siren {

Context::Context(const ContextDescriptor& descriptor) : m_descriptor(descriptor) {
    log::init(descriptor.level);
    glfwSetErrorCallback([](i32 err, const char* desc) {
        PANIC("GLFW error encountered. code: {}, description: {}", err, desc);
    });

    ASSERT(glfwInit(), "could not initialize GLFW.");

    time::initialize();

    // handle glfw setup
    switch (descriptor.backend) {
        case Backend::Auto: {
            log::info("Autoselecting a backend.");
            #if defined(SIREN_LINUX) || defined(SIREN_WINDOWS)
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            m_descriptor.backend = Backend::OpenGL;
            #elifdef SIREN_MACOS
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            #endif
        }
        case Backend::OpenGL: {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            break;
        }
        case Backend::Metal: {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            break;
        }
    }

    ThreadPool::init();

    // mount 2iREN to virtual filesystem
    const auto engine_root = Path{std::string{SIREN_ENGINE_ROOT}};
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
    ThreadPool::shutdown();
}

auto Context::create_device() const -> std::unique_ptr<Device> {
    switch (m_descriptor.backend) {
        case Backend::OpenGL: {
            #if defined(SIREN_LINUX) || defined(SIREN_WINDOWS)
            return std::make_unique<OpenGLDevice>();
            #elifdef SIREN_MACOS
            PANIC("cannot create an OpenGLDevice on apple environments.");
            #endif
        }
        case Backend::Metal: {
            #if defined(SIREN_LINUX) || defined(SIREN_WINDOWS)
            PANIC("cannot create a MetalDevice on non apple environments.");
            #elifdef SIREN_MACOS
            return std::make_unique<MetalDevice>();
            #endif
        }
        default: PANIC("cannot create a device. no backend has been selected.");
    }
}

auto Context::create_window(const WindowDescriptor& descriptor) const -> Window {
    return Window{descriptor};
}
} // namespace siren
