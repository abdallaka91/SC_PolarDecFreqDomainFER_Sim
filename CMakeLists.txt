cmake_minimum_required(VERSION 3.16)

# ===========================
# Project setup
# ===========================
project(REPLAY LANGUAGES C CXX)

# ===========================
# ENABLE ALL WARNINGS
# ===========================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release)

# ===========================
# Build type configuration
# ===========================

set(DEBUG_FLAGS -g3 -O0)
set(RELEASE_FLAGS -O3 -march=native -mtune=native -ffast-math -funroll-loops)
set(COMPILE_FLAGS $<$<CONFIG:Debug,RelWithDebInfo>:${DEBUG_FLAGS}>
                  $<$<CONFIG:Release,RelWithDebInfo>:${RELEASE_FLAGS}>
)
add_compile_options("${COMPILE_FLAGS}")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)

# ===========================
# Dependencies
# ===========================
find_package(Threads REQUIRED)
if(APPLE)
  message(NOTICE "-- Apple system targeted, resorting to hard coded OpenMP flags.")
  add_compile_options(-fopenmp)
  add_link_options(-fopenmp)
else()
  find_package(OpenMP REQUIRED)
endif()

# Simple FFTW3 detection (works on Ubuntu)
find_path(FFTW3_INCLUDE_DIR fftw3.h PATHS /usr/include /usr/local/include)

find_library(FFTW3_LIBRARY fftw3 PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

find_library(FFTW3_LIBRARY_float fftw3f PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

# Simple FFTW3F detection (works on Ubuntu)
find_path(FFTW3F_INCLUDE_DIR fftw3f.h PATHS /usr/include /usr/local/include)

find_library(FFTW3F_LIBRARY fftw3f PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

if(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARY)
  set(FFTW3_FOUND TRUE)
  message(STATUS "Found FFTW3: ${FFTW3_LIBRARY}")
else()
  set(FFTW3_FOUND FALSE)
  message(WARNING "FFTW3 not found.")
endif()

# ===========================
# Source setup
# ===========================

file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/source/*.cpp")

add_library(ccsk_simulators OBJECT)
target_include_directories(ccsk_simulators PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(ccsk_simulators PUBLIC ${FFTW3_INCLUDE_DIR})
target_link_libraries(ccsk_simulators PUBLIC ${FFTW3_LIBRARY} ${FFTW3_LIBRARY_float})

foreach(GF 64 128 256 512 1024)
  foreach(N 8 16 32 64 128 256 512 1024 2048)
    configure_file(
      ccsk_simulator/ccsk_simulator_implementations.cpp.in ccsk_stub_${GF}_${N}.cpp @ONLY
    )
    target_sources(ccsk_simulators PRIVATE ccsk_stub_${GF}_${N}.cpp)
  endforeach()
endforeach()

add_executable(Sim2 ${SRC_FILES} ${CMAKE_SOURCE_DIR}/decoders_simulator.cpp)

set_target_properties(Sim2 PROPERTIES BUILD_RPATH "${CMAKE_SOURCE_DIR}")

# ===========================
# Include Directories
# ===========================
target_include_directories(
  Sim2 PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/ccsk_simulator
              ${CMAKE_SOURCE_DIR}/ccsk_simulator/aff3ct_randn_gen
)

# ===========================
# Link Libraries
# ===========================
target_link_libraries(Sim2 PRIVATE Threads::Threads ccsk_simulators NbScFFTdec)
target_link_directories(Sim2 PRIVATE ${CMAKE_SOURCE_DIR})
if(NOT APPLE)
  target_link_libraries(Sim2 PRIVATE OpenMP::OpenMP_CXX)
endif()

add_executable(replay ${SRC_FILES} ${CMAKE_SOURCE_DIR}/replay_simulator.cpp)

# ===========================
# Include Directories
# ===========================
target_include_directories(
  replay PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/ccsk_simulator
                ${CMAKE_SOURCE_DIR}/ccsk_simulator/aff3ct_randn_gen
)

# Option for ALL_GFs
option(USE_ALL_GFs "Enable ALL_GFs compilation" OFF)
message(STATUS "USE_ALL_GFs value: ${USE_ALL_GFs}")
if(USE_ALL_GFs)
  target_compile_definitions(replay PUBLIC ALL_GFs)
  message(STATUS "ALL_GFs compilation enabled")
else()
  message(STATUS "ALL_GFs compilation disabled")
endif()

# ===========================
# Link Libraries
# ===========================
target_link_libraries(replay PRIVATE Threads::Threads ccsk_simulators)
if(NOT APPLE)
  target_link_libraries(replay PRIVATE OpenMP::OpenMP_CXX)
endif()

# ===========================
# Diagnostics
# ===========================
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Flags    : ${CMAKE_CXX_FLAGS}")

####################################################################################################
# INSTALL SECTION
####################################################################################################
