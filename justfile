set default-list

_configure type:
    conan install .                                     \
        --output-folder=build                           \
        --build=missing                                 \
        -s build_type={{ capitalize(type) }}            \
        -c tools.cmake.cmaketoolchain:generator=Ninja

    cmake --preset conan-{{ lowercase(type) }}          \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON              \

_build type:
    cmake --build --preset conan-{{ lowercase(type) }}

_example target path:
    cmake --build --preset conan-release --target {{ target }}
    {{ path }}


# Downloads dependencies and configure 2iREN for Release mode.
configure: (_configure "release")

# Builds 2iREN in Release mode.
build: (_build "release")

# Downloads dependencies and configure 2iREN for Debug mode.
configure-debug: (_configure "debug")

# Builds 2iREN in Debug mode.
build-debug: (_build "debug")

# Runs the 2iREN tests.
test: build
    ctest --preset conan-release --output-on-failure

# Runs the hello_triangle example.
example-hello: (_example "hello_triangle" "./build/build/Release/examples/01_hello_triangle/hello_triangle")

# Runs the spinning_cube example.
example-cube: (_example "spinning_cube" "./build/build/Release/examples/02_spinning_cube/spinning_cube")

# Runs the load_shader example.
example-shader: (_example "load_shader" "./build/build/Release/examples/03_load_shader/load_shader")

# Runs the load_gltf example.
example-gltf: (_example "load_gltf" "./build/build/Release/examples/04_load_gltf/load_gltf")

