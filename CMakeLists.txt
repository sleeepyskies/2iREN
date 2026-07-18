cmake_minimum_required(VERSION 3.30)
project(2iren)

# create compile_commands.json, needed for nvim navigation
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# warnings as errors
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_compile_options(
        # todo: research more what flags there are, could b some more useful ones idk
        -Wall               # activate multiple reliable warnings
        -Wextra             # similar to Wall, just more
        -Wpedantic          # warnings about non standard compiler extensions
        -Wnull-dereference  # identifies some null dereference cases
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(
        2iren
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

        src/2iren/window.cpp

        src/2iren/util/time.cpp
        src/2iren/util/filesystem.cpp
        src/2iren/util/stb_image.cpp
        src/2iren/util/cgltf.cpp
        src/2iren/util/camera.cpp
)

target_include_directories(2iren PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# pass root dir for VFS of 2iren to work
target_compile_definitions(2iren PRIVATE SIREN_ENGINE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}")

include(dependencies.cmake)
include(examples.cmake) # todo: optionally include examples

