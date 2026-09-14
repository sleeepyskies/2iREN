set default-list

export DYLD_INSERT_LIBRARIES := "/usr/lib/libMTLCapture.dylib:/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/Library/GPUToolsPlatform/libMTLToolsDiagnostics.dylib"
export METAL_DEBUG_ERROR_MODE := "0"
export METAL_DEVICE_FORCE_COMMAND_BUFFER_ENHANCED_ERRORS := "1"
export METAL_DEVICE_WRAPPER_TYPE := "5"
export METAL_DIAGNOSTICS_ENABLED := "1"
export METAL_LOAD_INTERPOSER := "1"
export MTL_FORCE_COMMAND_BUFFER_ENHANCED_ERRORS := "1"
export DYMTL_TOOLS_DYLIB_PATH := "/usr/lib/libMTLCapture.dylib"
export MTL_DEBUG_LAYER := "1"
export MTL_SHADER_VALIDATION := "1"

_configure type:
    conan install .                                     \
        --build=missing                                 \
        -s compiler.cppstd=23                           \
        -s build_type={{ capitalize(type) }}            \
        -c tools.cmake.cmaketoolchain:generator=Ninja

    cmake --preset conan-{{ lowercase(type) }}

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
example-hello: (_example "hello_triangle" "./build/Release/examples/01_hello_triangle/hello_triangle")

# Runs the spinning_cube example.
example-cube: (_example "spinning_cube" "./build/Release/examples/02_spinning_cube/spinning_cube")

# Runs the load_shader example.
example-shader: (_example "load_shader" "./build/Release/examples/03_load_shader/load_shader")

# Runs the tetris example.
example-tetris: (_example "load_shader" "./build/Release/examples/04_tetris/tetris")

