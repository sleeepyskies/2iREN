#pragma once

#if defined(SIREN_LINUX) or defined(SIREN_WINDOWS)
#include <glad/gl.h>
#include <GL/gl.h>
#elifdef SIREN_MACOS
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#endif
