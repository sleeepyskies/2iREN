cmake_minimum_required(VERSION 3.24)
project(
        2iREN
        VERSION 0.1.0
        LANGUAGES CXX
        DESCRIPTION "A C++23 Rendering Framework"
)

option(SIREN_BUILD_EXAMPLES
    "Build 2iREN examples"
    ${PROJECT_IS_TOP_LEVEL}
)

option(SIREN_BUILD_TESTS
    "Build 2iREN tests"
    ${PROJECT_IS_TOP_LEVEL}
)

option(SIREN_SINGLE_THREADED
    "Compile 2iREN without multithreading"
    ON
)

add_library(2iREN STATIC)
add_library(2iREN::2iREN ALIAS 2iREN)

# TODO: clean this up, CMakeLists.txt for each module please
target_sources(2iREN PRIVATE
    src/2iREN/rhi/backend/gl/command_executor.cpp
    src/2iREN/rhi/backend/gl/device.cpp
    src/2iREN/rhi/backend/gl/debug.cpp
    src/2iREN/rhi/backend/gl/render_thread.cpp

    src/2iREN/rhi/resources/buffer.cpp
    src/2iREN/rhi/resources/graphics_pipeline.cpp
    src/2iREN/rhi/resources/image.cpp
    src/2iREN/rhi/resources/sampler.cpp
    src/2iREN/rhi/resources/shader.cpp
    src/2iREN/rhi/resources/swapchain.cpp
    src/2iREN/rhi/resources/query.cpp

    src/2iREN/context.cpp
    src/2iREN/rhi/layout.cpp
    src/2iREN/rhi/render_command.cpp
    src/2iREN/rhi/resource_command.cpp

    src/2iREN/sync/thread_pool.cpp

    src/2iREN/asset/asset_path.cpp
    src/2iREN/asset/asset_server.cpp
    src/2iREN/asset/assets/shader.cpp
    src/2iREN/asset/assets/gltf.cpp
    src/2iREN/asset/assets/material.cpp
    src/2iREN/asset/assets/texture.cpp

    src/2iREN/input/input.cpp
    src/2iREN/window.cpp

    src/2iREN/util/time.cpp
    src/2iREN/util/filesystem.cpp
    src/2iREN/util/stb_image.cpp
    src/2iREN/util/stb_image_write.cpp
    src/2iREN/util/cgltf.cpp
    src/2iREN/util/camera.cpp
)

target_include_directories(
        2iREN
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
            $<INSTALL_INTERFACE:include>
)

target_compile_definitions(
        2iREN
        PUBLIC
            GLFW_INCLUDE_NONE
            SIREN_SINGLE_THREADED=$<BOOL:${SIREN_SINGLE_THREADED}>
)

target_compile_features(2iREN PUBLIC cxx_std_23)

set_target_properties(2iREN PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

target_compile_options(2iREN PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Werror
        -Wconversion-null
        -Wmissing-declarations
        -Woverlength-strings
        -Wpointer-arith
        -Wunused-local-typedefs
        -Wunused-result
        -Wvarargs
        -Wvla
        -Wwrite-strings
)

include(dependencies.cmake)

if(SIREN_BUILD_EXAMPLES)
    message(STATUS "Building 2iREN Examples...")
    add_subdirectory(examples/01_hello_triangle)
    add_subdirectory(examples/02_spinning_cube)
    add_subdirectory(examples/03_load_shader)
    add_subdirectory(examples/04_load_gltf)
endif()

if(SIREN_BUILD_TESTS)
    enable_testing()
    message(STATUS "Building 2iREN tests")
    add_subdirectory(tests)
endif()
