target_sources(2iREN PRIVATE
    layout.cpp
    render_command.cpp
    resource_command.cpp

    buffer.cpp
    graphics_pipeline.cpp
    image.cpp
    query.cpp
    sampler.cpp
    shader.cpp
    swapchain.cpp
)

add_subdirectory(backend/gl)
