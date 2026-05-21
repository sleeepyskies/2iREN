cmake_minimum_required(VERSION 3.30)
project(2iren)

# warnings as errors
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
add_compile_options(
        -Wall -Wextra -Wpedantic
        -Werror
        -Wshadow -Wconversion -Wsign-conversion
        -Wnull-dereference -Wdouble-promotion
        -Wformat=2
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
        src/2iren/rhi/resources/framebuffer.cpp
        src/2iren/rhi/resources/graphics_pipeline.cpp
        src/2iren/rhi/resources/image.cpp
        src/2iren/rhi/resources/sampler.cpp
        src/2iren/rhi/resources/shader.cpp
        src/2iren/rhi/resources/swapchain.cpp

        src/2iren/rhi/context.cpp
        src/2iren/rhi/layout.cpp
        src/2iren/rhi/render_command.cpp
        src/2iren/rhi/resource_command.cpp

        src/2iren/window.cpp
)

target_include_directories(2iren PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(dependencies.cmake)
include(examples.cmake)

