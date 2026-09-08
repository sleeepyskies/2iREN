#include "2iREN/core/assert.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

#define STBIW_ASSERT(x) ASSERT(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb/stb_image_write.h"

#pragma clang diagnostic pop

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
