target_sources(2iREN PRIVATE
    layout.cpp
    render_command.cpp

    buffer.cpp
    graphics_pipeline.cpp
    image.cpp
    query.cpp
    sampler.cpp
    shader.cpp
    swapchain.cpp
)

add_subdirectory(backend/opengl)
add_subdirectory(backend/metal)
