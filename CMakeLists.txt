cmake_minimum_required(VERSION 3.24)
project(
        2iREN
        VERSION 0.1.0
        LANGUAGES CXX
        DESCRIPTION "A c++23 Rendering Framework"
)

option(SIREN_BUILD_EXAMPLES "Build 2iREN Examples" ${PROJECT_IS_TOP_LEVEL})
option(SIREN_BUILD_TESTS "Build 2iREN Tests" ${PROJECT_IS_TOP_LEVEL})
option(SIREN_SINGLE_THREADED "Compile 2iREN to run without multi-threading" ON)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

add_library(
        # todo: clean this up?
        2iREN
        STATIC
        src/2iren/rhi/backend/gl/command_executor.cpp
        src/2iren/rhi/backend/gl/device.cpp
        src/2iren/rhi/backend/gl/debug.cpp
        src/2iren/rhi/backend/gl/render_thread.cpp

        src/2iren/rhi/resources/buffer.cpp
        src/2iren/rhi/resources/graphics_pipeline.cpp
        src/2iren/rhi/resources/image.cpp
        src/2iren/rhi/resources/sampler.cpp
        src/2iren/rhi/resources/shader.cpp
        src/2iren/rhi/resources/swapchain.cpp
        src/2iren/rhi/resources/query.cpp

        src/2iren/rhi/context.cpp
        src/2iren/rhi/layout.cpp
        src/2iren/rhi/render_command.cpp
        src/2iren/rhi/resource_command.cpp
        src/2iren/rhi/render_target.hpp

        src/2iren/sync/thread_pool.cpp

        src/2iren/asset/asset_path.cpp
        src/2iren/asset/asset_server.cpp
        src/2iren/asset/assets/shader.cpp
        src/2iren/asset/assets/gltf.cpp
        src/2iren/asset/assets/material.cpp
        src/2iren/asset/assets/texture.cpp

        src/2iren/input/input.cpp

        src/2iren/window.cpp

        src/2iren/util/time.cpp
        src/2iren/util/filesystem.cpp
        src/2iren/util/stb_image.cpp
        src/2iren/util/stb_image_write.cpp
        src/2iren/util/cgltf.cpp
        src/2iren/util/camera.cpp
)

add_library(2iREN::2iREN ALIAS 2iREN)

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


set_target_properties(2iREN PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
target_compile_features(2iREN PUBLIC cxx_std_23)
target_compile_options(2iREN PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wnull-dereference
)

include(dependencies.cmake)

if (SIREN_BUILD_EXAMPLES)
    message(STATUS "Building 2iREN Examples...")
    add_subdirectory(examples/01_hello_triangle)
    add_subdirectory(examples/02_spinning_cube)
    add_subdirectory(examples/03_load_shader)
    add_subdirectory(examples/04_load_gltf)
endif ()

if (SIREN_BUILD_TESTS)
    message(STATUS "Running 2iREN Tests...")
endif ()
