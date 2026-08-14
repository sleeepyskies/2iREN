# 2iren

2iren is a simple cpp23 rendering library extracted and modified from the [siren](https://github.com/sleeepyskies/siren)
game engine.

Currently only OpenGL 4.6 is supported, but other APIs may be supported in the future.

## Building 2iREN



## Dependencies

Conan is used for package management.

* **stb_image**: Single header file used for image loading.
* **cgltf**: Loading gltf files.
* **yaml-cpp**: Used for certain 2iren specific file types (sshg etc.).
* **GLFW**: Windowing
* **GLM**: Math library
* **libassert**: Runtime assertions with stacktrace output
* **glad**: OpenGL loader
* **OpenGL**: Rendering API

# Examples

2iren includes a set of examples to show how to use the library. These can be found under `/examples`.

For a more detailed look into how 2iREN can be used as a framework, checkout oiter.

1. Rainbow Triangle

![tri.png](images/tri.png)

2. Spinning Cube

![cube.png](images/cube.png)

3. Load Shader

Demos loading a simple asset from the VFS. Also shows the yaml like file type for defining 2iren shaders.

4. Load Gltf (WIP)

Demos loading a mesh from a gltf file using the asset server and rendering it.