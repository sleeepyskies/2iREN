cmake_minimum_required(VERSION 3.30)
project(load_shader)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
        load_shader
        main.cpp
)

target_link_libraries(load_shader PRIVATE 2iren)

target_compile_definitions(2iren PUBLIC GLFW_INCLUDE_NONE)
