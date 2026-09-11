cmake_minimum_required(VERSION 3.24)
project(
        2iREN
        VERSION 0.1.0
        LANGUAGES CXX
        DESCRIPTION "A C++23 Graphics Framework"
)

option(SIREN_BUILD_EXAMPLES
    "Build 2iREN examples"
    ${PROJECT_IS_TOP_LEVEL}
)

option(SIREN_BUILD_TESTS
    "Build 2iREN tests"
    ${PROJECT_IS_TOP_LEVEL}
)

# required for some lsp's to work.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(2iREN STATIC)
add_library(2iREN::2iREN ALIAS 2iREN)

include(cmake/platform.cmake)
include(cmake/config.cmake)
include(cmake/dependencies.cmake)

add_subdirectory(src/2iREN/asset)
add_subdirectory(src/2iREN/concurrency)
add_subdirectory(src/2iREN/core)
add_subdirectory(src/2iREN/graphics)
add_subdirectory(src/2iREN/impl)
add_subdirectory(src/2iREN/window)
add_subdirectory(src/2iREN/scene)
add_subdirectory(src/2iREN/utility)

target_include_directories(2iREN PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

target_compile_definitions(2iREN
        PUBLIC
# TODO: really, we should just have an embedded asset loader, remove the whole
# VFS thing and just query the hidden embedded manager for core assets.
    PRIVATE
        SIREN_ENGINE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
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

if(SIREN_BUILD_EXAMPLES)
    message(STATUS "Building 2iREN examples")
    add_subdirectory(examples/01_hello_triangle)
    add_subdirectory(examples/02_spinning_cube)
    add_subdirectory(examples/03_load_shader)
    add_subdirectory(examples/04_tetris)
endif()

if(SIREN_BUILD_TESTS)
    enable_testing()
    message(STATUS "Building 2iREN tests")
    add_subdirectory(tests)
endif()
