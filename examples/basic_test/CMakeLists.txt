cmake_minimum_required(VERSION 3.30)
project(basic_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
        basic_test
        main.cpp
)

configure_file(
    ${CMAKE_SOURCE_DIR}/examples/basic_test/config.toml
    ${CMAKE_CURRENT_BINARY_DIR}/config.toml
    COPYONLY
)

target_link_libraries(basic_test PRIVATE 2iren)
