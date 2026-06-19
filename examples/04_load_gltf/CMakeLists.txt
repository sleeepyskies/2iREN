cmake_minimum_required(VERSION 3.30)
project(load_gltf)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
        load_gltf
        main.cpp
)

target_link_libraries(load_gltf PRIVATE 2iren)

target_compile_definitions(2iren PUBLIC GLFW_INCLUDE_NONE)
