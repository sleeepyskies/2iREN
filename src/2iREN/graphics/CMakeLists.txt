target_sources(2iREN PRIVATE
    buffer.cpp
    graphics_pipeline.cpp
    image.cpp
    query.cpp
    sampler.cpp
    shader.cpp
    swapchain.cpp
    layout.cpp
)

add_subdirectory(backend/opengl)
add_subdirectory(backend/metal)
