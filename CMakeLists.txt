cmake_minimum_required(VERSION 3.15)
project(wvw_killstreak VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type option: ARCDPS or NEXUS
option(BUILD_TYPE "Build type: ARCDPS or NEXUS" "NEXUS")

# Windows-specific settings
if(WIN32)
    if(BUILD_TYPE STREQUAL "ARCDPS")
        message(STATUS "Building ArcDPS addon")

        # Build as shared library (DLL)
        add_library(arcdps_killstreak SHARED
            killstreak.cpp
            arcdps_structs.h
        )

        # Set output name to match ArcDPS plugin naming convention
        set_target_properties(arcdps_killstreak PROPERTIES
            OUTPUT_NAME "arcdps_killstreak"
            PREFIX ""
            SUFFIX ".dll"
        )

        # Link Windows libraries
        target_link_libraries(arcdps_killstreak PRIVATE kernel32)

        set(TARGET_NAME arcdps_killstreak)
    else()
        message(STATUS "Building Nexus addon")

        # Build as shared library (DLL) for Nexus
        add_library(nexus_streamlink SHARED
            killstreak_nexus.cpp
            Nexus.h
            ArcDPS.h
        )

        # Set output name for Nexus addon
        # Nexus addons are placed in <GW2>/addons/ folder
        set_target_properties(nexus_streamlink PROPERTIES
            OUTPUT_NAME "nexus_streamlink"
            PREFIX ""
            SUFFIX ".dll"
        )

        # Link Windows libraries
        target_link_libraries(nexus_streamlink PRIVATE kernel32)

        set(TARGET_NAME nexus_streamlink)
    endif()

    # Disable C++ exceptions and RTTI for smaller binary
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /W4         # Warning level 4
            /WX-        # Don't treat warnings as errors
            /GR-        # Disable RTTI
            /EHs-c-     # Disable exceptions
            /MT         # Static runtime (no MSVCRT dependency)
        )
        target_compile_definitions(${TARGET_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
            WIN32_LEAN_AND_MEAN
            NOMINMAX
        )
    endif()

    # MinGW support
    if(MINGW)
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -fno-exceptions
            -fno-rtti
        )
        target_link_options(${TARGET_NAME} PRIVATE
            -static-libgcc
            -static-libstdc++
            -static
        )
    endif()
endif()

# Install target
install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
