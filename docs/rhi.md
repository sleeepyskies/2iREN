# Info

This document contains information about the RHI module of 2iREN.
It aims to document and explain how best to use the api.

## Support

Currently, 2iREN supports OpenGL and Metal, as well as glsl and msl
shading languages.

Eventually, OpenGL will be fully replaced by Vulkan.

## The API

### Objects

#### Device

The Device is the entry point for any Gpu related actions. It is responsible for:

- Resource Management (creation and destruction)
- Resource Information (inspecting descriptors)
- Creating and submitting command buffers.
- Swapchain presentation

#### CommandBuffer

#### RenderCommandEncoder

### Render Resources

blabla

### Binding Model

Blabla explain what binds how.
