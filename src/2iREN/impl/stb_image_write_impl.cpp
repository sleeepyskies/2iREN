#include "2iREN/core/assert.hpp"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#endif

#define STBIW_ASSERT(x) ASSERT(x)
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <stb_image_write.h>

#if defined(__clang__)
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#endif
