cmake_minimum_required(VERSION 3.16)

# ===========================
# Project setup
# ===========================
project(PolarDecoder LANGUAGES CXX)

# ===========================
# Build type configuration
# ===========================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Safe optimization that works everywhere
    set(CMAKE_CXX_FLAGS "-O3 -march=haswell -mtune=haswell -flto -ffast-math -funroll-loops -fno-math-errno")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ===========================
# Compiler options
# ===========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Silence warnings
add_compile_options(
    -Wno-unused-parameter
    -Wno-deprecated-copy
    -Wno-int-in-bool-context
    -Wno-type-limits
    -Wno-conversion
    -Wno-error=conversion
    -Wno-aggressive-loop-optimizations
    -Wno-strict-aliasing
)

# Safe SIMD for both machines (AVX2 works everywhere)
add_compile_options(-mavx2 -mfma -mbmi2)
add_compile_options(-ftree-vectorize -fvect-cost-model=unlimited)

# ===========================
# Dependencies
# ===========================
find_package(OpenMP REQUIRED)

# ===========================
# Source files
# ===========================
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/utilities/**.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/specialized_pruning/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/specialized/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/dedicated/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/basic/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_fixed/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_cfloat/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_integer/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_int32_t/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/hadamard/Hadamard.cpp"
    "${CMAKE_SOURCE_DIR}/decoders_simulator.cpp"
)

add_executable(Sim2 ${SRC_FILES})

# ===========================
# Include directories
# ===========================
target_include_directories(Sim2 PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/BLG/src/include
    ${CMAKE_SOURCE_DIR}/BLG/src/decoders
    ${CMAKE_SOURCE_DIR}/BLG/src/definitions
    ${CMAKE_SOURCE_DIR}/BLG/src/hadamard
    ${CMAKE_SOURCE_DIR}/BLG/src/utilities
    ${CMAKE_SOURCE_DIR}/BLG/src
    ${CMAKE_SOURCE_DIR}/BLG/src/features/fwht
)

# ===========================
# Optional features
# ===========================
option(USE_ALL_GFs "Enable ALL_GFs compilation" OFF)
if(USE_ALL_GFs)
    target_compile_definitions(Sim2 PUBLIC ALL_GFs)
endif()

# ===========================
# Link libraries
# ===========================
target_link_libraries(Sim2 PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(Sim2 PRIVATE m)

# Link-time optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
endif()

# ===========================
# Performance diagnostics
# ===========================
message(STATUS "========================================")
message(STATUS "Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "========================================")