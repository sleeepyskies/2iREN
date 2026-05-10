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

add_library(2iren STATIC
    src/2iren/render_command.cpp
    src/2iren/render_thread.cpp
    src/2iren/resource_command.cpp
    src/2iren/vertex_layout.cpp
    src/2iren/device.cpp

    src/2iren/resources/buffer.cpp
    src/2iren/resources/framebuffer.cpp
    src/2iren/resources/graphics_pipeline.cpp
    src/2iren/resources/image.cpp
    src/2iren/resources/sampler.cpp
    src/2iren/resources/shader.cpp

    src/2iren/backend/gl/device.cpp
    src/2iren/backend/gl/command_executor.cpp
)

target_include_directories(2iren PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

include(dependencies.cmake)

# add any examples
add_subdirectory(examples/basic_test)

