#include "device.hpp"

#include <glad/gl.h>

#include "backend/gl/device.hpp"
#include "util/platform.hpp"
#include "util/log.hpp"


namespace siren {

static auto create_gl_device(const CreateDeviceParams& params) -> std::unique_ptr<Device> {
    log::info("Creating an OpenGlDevice.");
    gladLoadGL(glfwGetProcAddress);
    return std::make_unique<GlDevice>(params.window);
}

auto Device::create(const CreateDeviceParams& params) -> std::unique_ptr<Device> {
    switch (params.backend) {
        case Backend::Auto: {
            if constexpr (platform::current == platform::Windows) {
                return create_gl_device(params);
            } else {
                PANIC("Unsupported platform");
            }
        }
        case Backend::OpenGL: {
            return create_gl_device(params);
        }
        default: UNREACHABLE();
    }
}

} // namespace siren
