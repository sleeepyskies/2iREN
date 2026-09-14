# detect OS
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Detected Linux environment.")
    set(SIREN_LINUX ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_LINUX")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Detected Apple environment.")
    set(SIREN_MACOS ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_MACOS")
    enable_language(OBJCXX)
    set_target_properties(2iREN PROPERTIES
        OBJCXX_STANDARD 23
        OBJCXX_STANDARD_REQUIRED ON
        OBJCXX_EXTENSIONS OFF
    )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Detected Windows environment.")
    set(SIREN_WINDOWS ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_WINDOWS")
else()
    message(FATAL_ERROR, "Unsupported environment, aborting build.")
endif()

# detect compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Detected Clang Compiler.")
    set(SIREN_CLANG ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_CLANG")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Detected GCC Compiler.")
    set(SIREN_GCC ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_GCC")
endif()
