#pragma once

#include <glad/gl.h>


namespace siren::gl {

/**
 * @brief Callback that links OpenGL messages to 2iREN's logging system.
 */
auto debug_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam
) -> void;

} // namespace siren::gl
