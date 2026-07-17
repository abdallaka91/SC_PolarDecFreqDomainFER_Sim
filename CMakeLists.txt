cmake_minimum_required(VERSION 3.16)

# ===========================
#  Project setup
# ===========================
project(TEST1 LANGUAGES CXX)

add_compile_options(-Wno-unused-parameter -Wno-deprecated-copy -Wno-int-in-bool-context -Wno-type-limits -Wno-strict-aliasing -Wno-extra)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Generate the small BLG compile-time configuration without modifying the
# submodule's definitions/code.hpp. The generated header shadows it because
# its include directory is listed first below.
set(CODE_N 64 CACHE STRING "Polar code length compiled into BLG")
set(CODE_GF 64 CACHE STRING "Galois-field size compiled into BLG")

function(compute_power_of_two_log2 INPUT_VALUE OUTPUT_VARIABLE)
    if(NOT "${INPUT_VALUE}" MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "${OUTPUT_VARIABLE} source must be a positive integer, got '${INPUT_VALUE}'")
    endif()

    set(VALUE "${INPUT_VALUE}")
    set(LOG_VALUE 0)
    while(VALUE GREATER 1)
        math(EXPR REMAINDER "${VALUE} % 2")
        if(NOT REMAINDER EQUAL 0)
            message(FATAL_ERROR "${OUTPUT_VARIABLE} source must be a power of two, got '${INPUT_VALUE}'")
        endif()
        math(EXPR VALUE "${VALUE} / 2")
        math(EXPR LOG_VALUE "${LOG_VALUE} + 1")
    endwhile()

    set(${OUTPUT_VARIABLE} "${LOG_VALUE}" PARENT_SCOPE)
endfunction()

compute_power_of_two_log2("${CODE_N}" CODE_LOG_N)
compute_power_of_two_log2("${CODE_GF}" CODE_LOG_GF)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated/definitions")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/code.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/definitions/code.hpp"
    @ONLY
)
message(STATUS "Generated BLG code.hpp: N=${CODE_N}, GF=${CODE_GF}")

# ===========================
#  Build type configuration
# ===========================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build")
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build")
    set(CMAKE_CXX_FLAGS "-O3 -march=native -flto -ffast-math -funroll-loops -fno-math-errno -Wall -Wextra")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "No build type selected, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_CXX_FLAGS "-O3 -march=native -flto -ffast-math -funroll-loops -fno-math-errno -Wall -Wextra")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ===========================
#  Dependencies
# ===========================
find_package(OpenMP REQUIRED)

# Simple FFTW3 detection (works on Ubuntu)
find_path(FFTW3_INCLUDE_DIR fftw3.h
    PATHS /usr/include /usr/local/include
)

find_library(FFTW3_LIBRARY fftw3
    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib
)

if(NOT FFTW3_INCLUDE_DIR OR NOT FFTW3_LIBRARY)
    message(FATAL_ERROR "FFTW3 is required. Install with: sudo apt install libfftw3-dev")
endif()
message(STATUS "Found FFTW3: ${FFTW3_LIBRARY}")

# ===========================
#  Source setup
# ===========================
add_compile_options(-mavx2)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/source/**/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/utilities/**.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/hadamard/Hadamard.cpp"
    "${CMAKE_SOURCE_DIR}/genie_aided_sim.cpp"
)

add_executable(Sim2 ${SRC_FILES})

# ===========================
#  Include Directories
# ===========================
target_include_directories(Sim2 PUBLIC
    ${CMAKE_BINARY_DIR}/generated
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/BLG/src/include
    ${CMAKE_SOURCE_DIR}/BLG/src/decoders
    ${CMAKE_SOURCE_DIR}/BLG/src/definitions
    ${CMAKE_SOURCE_DIR}/BLG/src/hadamard
    ${CMAKE_SOURCE_DIR}/BLG/src/utilities
    ${CMAKE_SOURCE_DIR}/BLG/src
    ${CMAKE_SOURCE_DIR}/BLG/src/features/fwht
)

target_include_directories(Sim2 PUBLIC ${FFTW3_INCLUDE_DIR})

# Option for ALL_GFs
option(USE_ALL_GFs "Enable ALL_GFs compilation" OFF)
message(STATUS "USE_ALL_GFs value: ${USE_ALL_GFs}")
if(USE_ALL_GFs)
    target_compile_definitions(Sim2 PUBLIC ALL_GFs)
    message(STATUS "ALL_GFs compilation enabled")
else()
    message(STATUS "ALL_GFs compilation disabled")
endif()

# ===========================
#  Link Libraries
# ===========================
if(OpenMP_CXX_FOUND)
    target_link_libraries(Sim2 PRIVATE OpenMP::OpenMP_CXX)
endif()

target_link_libraries(Sim2 PRIVATE ${FFTW3_LIBRARY})
target_link_libraries(Sim2 PRIVATE m)

# ===========================
#  Diagnostics
# ===========================
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Flags    : ${CMAKE_CXX_FLAGS}")

#############################################################################
#single thread


# cmake_minimum_required(VERSION 3.16)

# # ===========================
# #  Project setup
# # ===========================
# project(TEST1 LANGUAGES CXX)

# # ===========================
# #  Configuration options
# # ===========================
# # option(FWHT_COUNTER_ENABLE "Enable FWHT call counter" ON)
# # message(STATUS "FWHT counter enabled: ${FWHT_COUNTER_ENABLE}")

# if(FWHT_COUNTER_ENABLE)
#     add_compile_definitions(FWHT_COUNTER_ENABLE=1)
#     message(STATUS "FWHT counter compilation is ENABLED")
# else()
#     add_compile_definitions(FWHT_COUNTER_ENABLE=0)
#     message(STATUS "FWHT counter compilation is DISABLED")
# endif()

# add_compile_options(-Wno-unused-parameter -Wno-deprecated-copy -Wno-int-in-bool-context -Wno-type-limits -Wno-strict-aliasing -Wno-extra)

# set(CMAKE_CXX_STANDARD 17)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# # ===========================
# #  Build type configuration
# # ===========================
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     message(STATUS "Configuring Debug build")
#     set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra")
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
# elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
#     message(STATUS "Configuring Release build")
#     set(CMAKE_CXX_FLAGS "-O3 -march=native -ffast-math -funroll-loops -fno-math-errno -Wall -Wextra")
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
# else()
#     message(STATUS "No build type selected, defaulting to Release")
#     set(CMAKE_BUILD_TYPE Release)
#     set(CMAKE_CXX_FLAGS "-O3 -march=native -ffast-math -funroll-loops -fno-math-errno -Wall -Wextra")
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
# endif()

# # ===========================
# #  Source setup
# # ===========================

# add_compile_options(-mavx2)

# set(SOURCE_DIR ${CMAKE_SOURCE_DIR})

# file(GLOB_RECURSE SRC_FILES
#     "${CMAKE_SOURCE_DIR}/source/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/utilities/**.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/specialized_pruning/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/specialized/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/dedicated/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/basic/*.cpp"    
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_fixed/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_cfloat/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/hadamard/Hadamard.cpp"
#     "${CMAKE_SOURCE_DIR}/single_frame_dec.cpp"
#     "${CMAKE_SOURCE_DIR}/genie_aided_sim.cpp"
#     )

# list(FILTER SRC_FILES EXCLUDE REGEX "simulator_dec_fft_B.cpp")
# list(FILTER SRC_FILES EXCLUDE REGEX "decoder_naive_int32_t")


# add_executable(Sim2 ${SRC_FILES})

# target_include_directories(Sim2 PUBLIC
#     ${CMAKE_SOURCE_DIR}/include
#      ${CMAKE_SOURCE_DIR}/BLG/src/include
#     ${CMAKE_SOURCE_DIR}/BLG/src/decoders
#     ${CMAKE_SOURCE_DIR}/BLG/src/definitions
#     ${CMAKE_SOURCE_DIR}/BLG/src/hadamard
#     ${CMAKE_SOURCE_DIR}/BLG/src/utilities
#     ${CMAKE_SOURCE_DIR}/BLG/src/
#     ${CMAKE_SOURCE_DIR}/BLG/src/features/fwht
# )

# # ===========================
# #  Optional diagnostics
# # ===========================
# message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER}")
# message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
# message(STATUS "C++ Flags    : ${CMAKE_CXX_FLAGS}")
