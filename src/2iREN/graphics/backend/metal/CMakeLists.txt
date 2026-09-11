if(SIREN_MACOS)
    message(STATUS "Compiling Metal backend.")
    target_sources(2iREN PRIVATE
        impl.cpp
        device.cpp
    )
endif()

