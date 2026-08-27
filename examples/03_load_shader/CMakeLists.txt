add_executable(
        load_shader
        main.cpp
)

target_link_libraries(
        load_shader
        PRIVATE
        2iREN::2iREN
)
