add_executable(
        hello_triangle
        main.cpp
)

target_link_libraries(
        hello_triangle
        PRIVATE
        2iREN::2iREN
)
