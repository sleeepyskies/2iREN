#include "context.hpp"

#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include "2iREN/concurrency/thread_pool.hpp"
#include "2iREN/graphics/device.hpp"
#include "2iREN/utility/filesystem.hpp"
#include "2iREN/utility/log.hpp"
#include "2iREN/utility/time.hpp"
#include "2iREN/window/window.hpp"

#if defined(SIREN_LINUX) || defined(SIREN_WINDOWS)
#include "2iREN/graphics/backend/opengl/device.hpp"
#elifdef SIREN_MACOS
#include "2iREN/graphics/backend/metal/device.hpp"
#endif

#ifndef SIREN_ENGINE_ROOT
#define SIREN_ENGINE_ROOT "."
#endif

namespace {

auto initialize_glfw() -> void {
    glfwSetErrorCallback([](siren::i32 err, const char* desc) {
        PANIC("GLFW error. code: {}, description: {}", err, desc);
    });

    ASSERT(glfwInit(), "could not initialize GLFW.");

    const auto platform = glfwGetPlatform();
    if (platform == GLFW_PLATFORM_X11) {
        siren::log::info("Using windowing platform X11");
    } else if (platform == GLFW_PLATFORM_WAYLAND) {
        siren::log::info("Using windowing platform Wayland");
    } else if (platform == GLFW_PLATFORM_COCOA) {
        siren::log::info("Using windowing platform Cocoa");
    } else if (platform == GLFW_PLATFORM_WIN32) {
        siren::log::info("Using windowing platform Win32");
    }

#if defined(SIREN_LINUX) || defined(SIREN_WINDOWS)
    siren::log::info("selecting OpenGL backend.");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#elifdef SIREN_MACOS
    siren::log::info("selecting Metal backend.");
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_backend = Backend::Metal;
#endif
}

} // namespace

namespace siren {

Context::Context(const ContextDescriptor& descriptor) {
    log::initialize(descriptor.level);

    time::initialize();

    ThreadPool::initialize();

    FileSystem::mount("engine", SIREN_ENGINE_ROOT);

    initialize_glfw();
}

auto Context::make(const ContextDescriptor& descriptor) -> Context {
    static bool called = false;
    ASSERT(!called, "a context has already been created");
    called = true;
    return Context{descriptor};
}

Context::~Context() {
    ThreadPool::shutdown();
    glfwTerminate();
}

auto Context::make_device() const -> std::unique_ptr<Device> {
    return
#if defined(SIREN_LINUX) || defined(SIREN_WINDOWS)
        std::make_unique<OpenGLDevice>();
#elifdef SIREN_MACOS
        std::make_unique<MetalDevice>();
#endif
}

auto Context::make_window(const WindowDescriptor& descriptor) const -> Window {
    static bool called = false;
    ASSERT(!called, "a window has already been created");
    called = true;
    return Window{descriptor};
}
} // namespace siren
