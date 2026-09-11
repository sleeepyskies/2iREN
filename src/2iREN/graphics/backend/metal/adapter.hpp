#pragma once

// forward decls
struct GLFWwindow;
namespace CA {
struct MetalLayer;
}

/// @brief Links the metal layer to the window. This will allow the metal device
/// to render into the window.
auto connect_to_window(GLFWwindow* glfw, CA::MetalLayer* layer) -> void;
