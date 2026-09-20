if(SIREN_MACOS)
    message(STATUS "Compiling Metal backend.")
    target_sources(2iREN PRIVATE
        device.cpp
        commands.cpp

        impl.cpp
        adapter.mm
    )
endif()

