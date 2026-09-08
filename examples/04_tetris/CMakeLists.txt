add_executable(
        tetris
        main.cpp
)

target_link_libraries(
        tetris
        PRIVATE
        2iREN::2iREN
)
