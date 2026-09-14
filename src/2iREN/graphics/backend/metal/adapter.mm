#import "adapter.hpp"

#import <AppKit/AppKit.h>
#import <GLFW/glfw3.h>
#import <GLFW/glfw3native.h>

namespace siren::metal {

auto connect_to_window(GLFWwindow* glfw, CA::MetalLayer* layer) -> void {
    CALayer* obj_layer = (__bridge CALayer*) layer;
    NSWindow* obj_window = glfwGetCocoaWindow(glfw);

    obj_window.contentView.layer      = obj_layer;
    obj_window.contentView.wantsLayer = YES;
}

} // namespace siren::metal

