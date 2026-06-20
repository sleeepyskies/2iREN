# brings in all of stb libraries. this isn't needed, as we only use stb_image actually

find_package(opengl_system REQUIRED)
find_package(glad REQUIRED)
find_package(libassert REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(cgltf REQUIRED)
find_package(stb REQUIRED)

target_link_libraries(2iren
        glad::glad
        opengl::opengl
        libassert::assert
        glm::glm
        glfw
        yaml-cpp::yaml-cpp
        cgltf::cgltf
        stb::stb
)
