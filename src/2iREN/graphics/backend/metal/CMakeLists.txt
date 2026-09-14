if(SIREN_MACOS)
    message(STATUS "Compiling Metal backend.")
    target_sources(2iREN PRIVATE
        device.cpp
        command_executor.cpp

        impl.cpp
        adapter.mm
    )
endif()

