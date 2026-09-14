#pragma once

#include "2iREN/graphics/backend/metal/fwd.hpp"

struct GLFWwindow;

namespace siren::metal {

/// @brief Links the metal layer to the window. This will allow the metal device
/// to render into the window.
auto connect_to_window(GLFWwindow* glfw, CA::MetalLayer* layer) -> void;

} // namespace siren::metal
