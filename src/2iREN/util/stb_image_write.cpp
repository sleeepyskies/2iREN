#include "2iREN/core/assert.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#define STBIW_ASSERT(x) ASSERT(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
