#include "debug.hpp"

#include <string>
#include <unordered_map>

#include "2iREN/base.hpp"
#include "2iREN/utility/log.hpp"

namespace siren::gl {

static auto source_to_string(const GLenum source) -> std::string {
    switch (source) {
        case GL_DEBUG_SOURCE_API: return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
        case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
        case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER:
        default: return "UNKNOWN";
    }
}

static auto type_to_string(const GLenum type) -> std::string {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
        case GL_DEBUG_TYPE_OTHER: return "OTHER";
        case GL_DEBUG_TYPE_MARKER: return "MARKER";
        default: return "UNKNOWN";
    }
}

static auto severity_to_string(const GLenum severity) -> std::string {
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
        case GL_DEBUG_SEVERITY_LOW: return "LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
        default: return "UNKNOWN";
    }
}

auto debug_callback(
    const GLenum source,
    const GLenum type,
    const GLuint id,
    const GLenum severity,
    const GLsizei length,
    const GLchar* message,
    const void* userParam
) -> void {
    (void)length;    // ignore unused
    (void)userParam; // ignore unused

    // limit to 5 repeats
    static std::unordered_map<u32, u32> count{};
    if (count[id] > 5) {
        return;
    }
    count[id]++;

    // source := where the error message comes from
    const auto source_string = source_to_string(source);
    const auto type_string   = type_to_string(type);
    const auto sev_string    = severity_to_string(severity);

    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        log::error(
            "OpenGL: [{} - {} ({})]: [{}] {}", sev_string, type_string, id, source_string, message
        );
    } else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
        log::warn(
            "OpenGL: [{} - {} ({})]: [{}] {}", sev_string, type_string, id, source_string, message
        );
    } else if (severity == GL_DEBUG_SEVERITY_LOW) {
        log::info(
            "OpenGL: [{} - {} ({})]: [{}] {}", sev_string, type_string, id, source_string, message
        );
    } else if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        log::trace(
            "OpenGL: [{} - {} ({})]: [{}] {}", sev_string, type_string, id, source_string, message
        );
    }
}

} // namespace siren::gl
