cmake_minimum_required(VERSION 3.16)

# ===========================
#  Project setup
# ===========================
project(TEST1 LANGUAGES CXX)

# option(DISABLE_FWHT_NORM "Disable FWHT normalization" ON)
# if (DISABLE_FWHT_NORM)
# 	add_compile_definitions (DISABLE_FWHT_NORM=1)
# endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For VSCode IntelliSense

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

# ===========================
#  Source setup
# ===========================

add_compile_options(-mavx2)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR})

# file(GLOB_RECURSE SRC_FILES
#     "${CMAKE_SOURCE_DIR}/source/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/utilities/**.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/src/decoders/**.cpp"
#     "${CMAKE_SOURCE_DIR}/simulator_dec_fft_A.cpp"
# )

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/utilities/**.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/specialized_pruning/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/specialized/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/dedicated/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/nb_sc_decoder/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_fixed/*.cpp"
    # "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_cfloat/*.cpp"
    # "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_int32_t/*.cpp"
    "${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_pruning/*.cpp"

    "${CMAKE_SOURCE_DIR}/simulator_dec_fft_A.cpp"
)

list(FILTER SRC_FILES EXCLUDE REGEX "simulator_dec_fft_B.cpp")
list(FILTER SRC_FILES EXCLUDE REGEX "decoder_naive_int32_t")

add_executable(Sim2 ${SRC_FILES})

target_include_directories(Sim2 PUBLIC
    ${CMAKE_SOURCE_DIR}/include
     ${CMAKE_SOURCE_DIR}/BLG/src/include
    ${CMAKE_SOURCE_DIR}/BLG/src/decoders
    ${CMAKE_SOURCE_DIR}/BLG/src/definitions
    ${CMAKE_SOURCE_DIR}/BLG/src/hadamard
    ${CMAKE_SOURCE_DIR}/BLG/src/utilities
    ${CMAKE_SOURCE_DIR}/BLG/src/
    ${CMAKE_SOURCE_DIR}/BLG/src/features/fwht
    # ${CMAKE_SOURCE_DIR}/BLG/src/decoders/naive_int32_t/
)

# ===========================
#  Linker configuration
# ===========================
if(OpenMP_CXX_FOUND)
    target_link_libraries(Sim2 PRIVATE OpenMP::OpenMP_CXX)
endif()

# ===========================
#  Optional diagnostics
# ===========================
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Flags    : ${CMAKE_CXX_FLAGS}")
