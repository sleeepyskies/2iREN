find_package(doctest REQUIRED)

add_executable(
    2iREN_tests
    test_main.cpp
    math_test.cpp
    color_test.cpp
)

target_link_libraries(
    2iREN_tests
    PRIVATE
        2iREN::2iREN
        doctest::doctest
)

add_test(
    NAME 2iREN.tests
    COMMAND 2iREN_tests
)
