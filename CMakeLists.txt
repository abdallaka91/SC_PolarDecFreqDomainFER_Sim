cmake_minimum_required(VERSION 3.16)

# ============================================================
# Project
# ============================================================
project(NB_POLAR_SIM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default build type: Release, but do NOT force it if user selects Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ============================================================
# Options
# ============================================================
option(USE_ALL_GFs "Enable ALL_GFs compilation for replay" OFF)
option(DISABLE_WARNINGS "Disable compiler warnings" ON)

# ============================================================
# Dependencies
# ============================================================
find_package(Threads REQUIRED)
find_package(OpenMP)

# FFTW3 detection
find_path(FFTW3_INCLUDE_DIR fftw3.h
    PATHS /usr/include /usr/local/include
)

find_library(FFTW3_LIBRARY fftw3
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
)

if(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARY)
    set(FFTW3_FOUND TRUE)
    message(STATUS "Found FFTW3: ${FFTW3_LIBRARY}")
else()
    set(FFTW3_FOUND FALSE)
    message(WARNING "FFTW3 not found. Install with: sudo apt install libfftw3-dev")
endif()

# ============================================================
# Common source files
# ============================================================
file(GLOB_RECURSE COMMON_SRC CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
)

# ============================================================
# Helper function for executables
# ============================================================
function(setup_decoder_target target_name main_file)

    add_executable(${target_name}
        ${COMMON_SRC}
        "${CMAKE_SOURCE_DIR}/${main_file}"
    )

    target_include_directories(${target_name} PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/ccsk_simulator
        ${CMAKE_SOURCE_DIR}/ccsk_simulator/aff3ct_randn_gen
    )

    set_target_properties(${target_name} PROPERTIES
        BUILD_RPATH "${CMAKE_SOURCE_DIR}"
    )

    target_link_libraries(${target_name} PRIVATE Threads::Threads)

    if(OpenMP_CXX_FOUND)
        target_link_libraries(${target_name} PRIVATE OpenMP::OpenMP_CXX)
    endif()

    if(FFTW3_FOUND)
        target_include_directories(${target_name} PUBLIC ${FFTW3_INCLUDE_DIR})
        target_compile_definitions(${target_name} PUBLIC USE_FFTW3)
        target_link_libraries(${target_name} PRIVATE ${FFTW3_LIBRARY} m ${CMAKE_DL_LIBS})
    endif()

    # ========================================================
    # Debug mode: deep debugging, good for F5/F10 stepping
    # ========================================================
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Debug>:
            -g3
            -ggdb3
            -O0
            -fno-omit-frame-pointer
            -fno-inline
            -fno-eliminate-unused-debug-types
        >
    )

    # Disable LTO in Debug
    set_property(TARGET ${target_name} PROPERTY
        INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE
    )

    # ========================================================
    # Release mode: very optimized
    # ========================================================
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:Release>:
            -O3
            -march=native
            -mtune=native
            -ffast-math
            -funroll-loops
            -fno-math-errno
            -flto
        >
    )

    target_link_options(${target_name} PRIVATE
        $<$<CONFIG:Release>:
            -flto
        >
    )

    set_property(TARGET ${target_name} PROPERTY
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    )

    # ========================================================
    # RelWithDebInfo: optimized but still debuggable
    # ========================================================
    target_compile_options(${target_name} PRIVATE
        $<$<CONFIG:RelWithDebInfo>:
            -O2
            -g3
            -ggdb3
            -fno-omit-frame-pointer
        >
    )

    set_property(TARGET ${target_name} PROPERTY
        INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO FALSE
    )

    # ========================================================
    # Optional warning suppression
    # ========================================================
    if(DISABLE_WARNINGS)
        target_compile_options(${target_name} PRIVATE -w)
    endif()

endfunction()

# ============================================================
# Executables
# ============================================================

setup_decoder_target(Sim2   decoders_simulator.cpp)
setup_decoder_target(replay replay_simulator.cpp)

if(USE_ALL_GFs)
    target_compile_definitions(replay PUBLIC ALL_GFs)
    message(STATUS "ALL_GFs compilation enabled for replay")
else()
    message(STATUS "ALL_GFs compilation disabled")
endif()

# ============================================================
# Diagnostics
# ============================================================
message(STATUS "C++ Compiler     : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type       : ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenMP found     : ${OpenMP_CXX_FOUND}")
message(STATUS "FFTW3 found      : ${FFTW3_FOUND}")
message(STATUS "USE_ALL_GFs      : ${USE_ALL_GFs}")
message(STATUS "Disable warnings : ${DISABLE_WARNINGS}")