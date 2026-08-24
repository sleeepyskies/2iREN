#include <libassert/assert.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"

#define STBIW_ASSERT(x) ASSERT(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#pragma clang diagnostic pop