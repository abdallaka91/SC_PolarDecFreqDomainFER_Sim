cmake_minimum_required(VERSION 3.16)

# ===========================
#  Project setup
# ===========================
project(TEST1 LANGUAGES CXX)

# ===========================
#  DISABLE ALL WARNINGS
# ===========================
# Nuclear option: disable ALL warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

# Additional suppression for extra safety
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wno-everything)
    add_compile_options(-Wno-all)
endif()

# Override any -Wall or -Wextra that might be added later
add_compile_options(
    -Wno-unused-parameter
    -Wno-deprecated-copy 
    -Wno-int-in-bool-context 
    -Wno-type-limits 
    -Wno-strict-aliasing 
    -Wno-extra
    -Wno-unused-but-set-variable
    -Wno-unused-variable
    -Wno-uninitialized
    -Wno-maybe-uninitialized
    -Wno-format
    -Wno-aggressive-loop-optimizations
    -Wno-unknown-pragmas
    -Wno-sign-compare
    -Wno-reorder
    -Wno-attributes
    -Wno-comment
    -Wno-pedantic
    -Wno-shadow
    -Wno-conversion
    -Wno-overloaded-virtual
    -Wno-narrowing
    -Wno-misleading-indentation
    -Wno-ignored-qualifiers
    -Wno-invalid-offsetof
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ===========================
#  Build type configuration
# ===========================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build")
    set(CMAKE_CXX_FLAGS "-g -O0 -w")  # Keep -w to disable warnings
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build")
    # set(CMAKE_CXX_FLAGS "-O3 -march=native -mtune=native -flto -ffast-math -funroll-loops -fno-math-errno -w -fopenmp")  # Keep -w
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native -flto -ffast-math -funroll-loops -fno-math-errno -w")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "No build type selected, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_CXX_FLAGS "-O3 -march=native -mtune=native -flto -ffast-math -funroll-loops -fno-math-errno -w -fopenmp")  # Keep -w
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Force disable warnings in ALL configurations
string(APPEND CMAKE_CXX_FLAGS " -w")

# ===========================
#  Dependencies
# ===========================
find_package(Threads REQUIRED) 

# Simple FFTW3 detection (works on Ubuntu)
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

# ===========================
#  Source setup
# ===========================

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/source/*.cpp"
    "${CMAKE_SOURCE_DIR}/decoders_simulator.cpp"
)

add_executable(Sim2 ${SRC_FILES})

# ===========================
#  Include Directories
# ===========================
target_include_directories(Sim2 PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ccsk_simulator
    ${CMAKE_SOURCE_DIR}/ccsk_simulator/aff3ct_randn_gen
)

if(FFTW3_FOUND)
    target_include_directories(Sim2 PUBLIC ${FFTW3_INCLUDE_DIR})
    target_compile_definitions(Sim2 PUBLIC USE_FFTW3)
endif()

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
target_link_libraries(Sim2 PRIVATE Threads::Threads)
if(OpenMP_CXX_FOUND)
    target_link_libraries(Sim2 PRIVATE OpenMP::OpenMP_CXX)
endif()

if(FFTW3_FOUND)
    target_link_libraries(Sim2 PRIVATE ${FFTW3_LIBRARY})
    # Link math library for exp(), cos(), etc.
    target_link_libraries(Sim2 PRIVATE m)
    target_link_libraries(Sim2 PRIVATE dl)
endif()

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
#     set(CMAKE_CXX_FLAGS "-g -O0 -Wall -Wextra  -fopenmp")
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
# elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
#     message(STATUS "Configuring Release build")
#     set(CMAKE_CXX_FLAGS "-O3 -march=native -ffast-math -funroll-loops -fno-math-errno -Wall -Wextra -fopenmp")
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
# else()
#     message(STATUS "No build type selected, defaulting to Release")
#     set(CMAKE_BUILD_TYPE Release)
#     set(CMAKE_CXX_FLAGS "-O3 -march=native -ffast-math -funroll-loops -fno-math-errno -Wall -Wextra  -fopenmp")
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
#     "${CMAKE_SOURCE_DIR}/decoders_simulator.cpp"
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
