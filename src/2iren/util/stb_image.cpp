#include <libassert/assert.hpp>

#define STBI_ASSERT(x) ASSERT(x)
#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
