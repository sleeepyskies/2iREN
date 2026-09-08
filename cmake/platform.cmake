if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Detected Linux environment.")

    set(SIREN_LINUX ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_LINUX")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Detected Apple environment.")

    set(SIREN_MACOS ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_MACOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(STATUS "Detected Windows environment.")

    set(SIREN_WINDOWS ON)
    target_compile_definitions(2iREN PUBLIC "SIREN_WINDOWS")
else()
    message(FATAL_ERROR, "Unsupported environment, aborting build.")
endif()
