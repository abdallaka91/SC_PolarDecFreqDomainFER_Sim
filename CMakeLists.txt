cmake_minimum_required(VERSION 3.10)

project(TEST1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Debug build settings
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "-g -O0")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()

# Release build settings
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "-O3 -march=native -flto -ffast-math -funroll-loops -fno-math-errno")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Enable OpenMP for Sim2 only
find_package(OpenMP REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)


add_executable(Sim2
    ${SOURCE_DIR}/simulator_dec_fft.cpp
    ${SOURCE_DIR}/GF_tools.cpp
    ${SOURCE_DIR}/init.cpp
    ${SOURCE_DIR}/tools.cpp
    ${SOURCE_DIR}/Decoder_functions.cpp
    ${SOURCE_DIR}/BLG/naive_pruning/decoder_naive_pruning.cpp
    ${SOURCE_DIR}/channel.cpp
)

if(OpenMP_CXX_FOUND)
    target_compile_options(Sim2 PRIVATE ${OpenMP_CXX_FLAGS})
    target_link_libraries(Sim2 PRIVATE OpenMP::OpenMP_CXX)
endif()