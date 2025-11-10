cmake_minimum_required(VERSION 3.16)

# ===========================
#  Project setup
# ===========================
project(TEST1 LANGUAGES CXX)

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

# add_compile_options(-mavx2)

# set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source/sim_cpp_sources)

# file(GLOB_RECURSE SRC_FILES
#     "${CMAKE_SOURCE_DIR}/source/sim_cpp_sources/*.cpp"
#     "${CMAKE_SOURCE_DIR}/BLG/**/*.cpp"
# )
# add_executable(Sim2 ${SRC_FILES})

# target_include_directories(Sim2 PRIVATE
#     ${CMAKE_SOURCE_DIR}/include
#     ${CMAKE_SOURCE_DIR}/source
#     ${CMAKE_SOURCE_DIR}/BLG
#     ${CMAKE_SOURCE_DIR}/BLG/decoders
#     ${CMAKE_SOURCE_DIR}/BLG/decoders/features
# )

# ===========================
#  Linker configuration
# ===========================
# if(OpenMP_CXX_FOUND)
#     target_link_libraries(Sim2 PRIVATE OpenMP::OpenMP_CXX)
# endif()


# target_compile_definitions(Sim1 PRIVATE ABD_OPTIM)
#=============================================================
add_compile_options(-mavx2)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/source/sim_cpp_sources)
set(BLG_DIR ${CMAKE_SOURCE_DIR}/BLG)

# Common source files (exclude mains)
file(GLOB_RECURSE COMMON_SRC
    "${SRC_DIR}/*.cpp"
    "${BLG_DIR}/**/*.cpp"
)
# Remove main files from common sources
list(FILTER COMMON_SRC EXCLUDE REGEX "simulator_dec_fft_[AB]\\.cpp$")

# Executable 1: SimA
add_executable(SimA
    ${COMMON_SRC}
    "${SRC_DIR}/simulator_dec_fft_A.cpp"
)
target_include_directories(SimA PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/source
    ${CMAKE_SOURCE_DIR}/BLG
    ${CMAKE_SOURCE_DIR}/BLG/decoders
    ${CMAKE_SOURCE_DIR}/BLG/decoders/features
)
target_compile_definitions(SimA PRIVATE ABD_OPTIM)

# Executable 2: SimB
add_executable(SimB
    ${COMMON_SRC}
    "${SRC_DIR}/simulator_dec_fft_B.cpp"
)
target_include_directories(SimB PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/source
    ${CMAKE_SOURCE_DIR}/BLG
    ${CMAKE_SOURCE_DIR}/BLG/decoders
    ${CMAKE_SOURCE_DIR}/BLG/decoders/features
)
target_compile_definitions(SimB PRIVATE ABD_OPTIM)

if(OpenMP_CXX_FOUND)
    target_link_libraries(SimA PRIVATE OpenMP::OpenMP_CXX)
endif()

if(OpenMP_CXX_FOUND)
    target_link_libraries(SimB PRIVATE OpenMP::OpenMP_CXX)
endif()






# ===========================
#  Optional diagnostics
# ===========================
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Flags    : ${CMAKE_CXX_FLAGS}")
