find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(glfw3 REQUIRED)

target_compile_definitions(2iREN PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(
        2iREN

        PRIVATE
            Threads::Threads
            yaml-cpp::yaml-cpp
            glfw
)

if(SIREN_LINUX OR SIREN_WINDOWS)
    find_package(opengl_system REQUIRED)
    find_package(glad REQUIRED)

    target_link_libraries(
            2iREN
            PRIVATE
                glad::glad
                opengl::opengl
                stdc++exp # for stacktrace TODO: should enable only for gcc
    )
elseif(SIREN_MACOS)
    find_package(metal-cpp REQUIRED)

    target_link_libraries(
            2iREN
            PRIVATE
                metal-cpp::metal-cpp
    )
endif()


