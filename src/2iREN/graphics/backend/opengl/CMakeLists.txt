if(NOT SIREN_MACOS)
    message(STATUS "Compiling OpenGL backend.")
    target_sources(2iREN PRIVATE
        command_executor.cpp
        debug.cpp
        device.cpp
    )
endif()

