find_package(opengl_system REQUIRED)
find_package(glad REQUIRED)
find_package(libassert REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(
        2iREN
        PUBLIC
            glfw
            glm::glm
            libassert::assert

        PRIVATE
            glad::glad
            opengl::opengl
            yaml-cpp::yaml-cpp
            Threads::Threads
)
