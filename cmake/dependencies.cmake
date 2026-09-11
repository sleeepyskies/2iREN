find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(glfw3 REQUIRED)
find_package(cgltf REQUIRED)
find_package(stb REQUIRED)

target_compile_definitions(2iREN PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(
    2iREN
    PUBLIC
        stb::stb

    PRIVATE
        Threads::Threads
        yaml-cpp::yaml-cpp
        cgltf::cgltf
        glfw
)

if(SIREN_GCC)
    # used for stacktrace, gcc only supports in experimental,
    # clang does not support
    target_link_libraries(2iREN PRIVATE stdc++exp)
endif()

if(SIREN_LINUX OR SIREN_WINDOWS)
    find_package(opengl_system REQUIRED)
    find_package(glad REQUIRED)

    target_link_libraries(2iREN PRIVATE glad::glad opengl::opengl)
elseif(SIREN_MACOS)
    find_package(metal-cpp REQUIRED)

    target_link_libraries(2iREN PRIVATE metal-cpp::metal-cpp)

    # needed to link device to window
    target_compile_definitions(2iREN PRIVATE GLFW_EXPOSE_NATIVE_COCOA)
endif()


