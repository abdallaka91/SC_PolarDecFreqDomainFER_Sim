cmake_minimum_required(VERSION 3.21)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

project(NB_SC_DECODER_EXPLORER LANGUAGES CXX)


add_compile_options (-march=native -mtune=native -O3 -ffast-math)
add_compile_options (-Wall -Wextra)

#######################################################################################################################################

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-gnu-inline-cpp-without-extern -march=native -mtune=native -O3 -ffast-math -g3")

add_executable (nb_sc_decoder ${CMAKE_CURRENT_SOURCE_DIR}/apps/nb_sc_decoder.cpp)
target_include_directories (nb_sc_decoder PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common)

#######################################################################################################################################

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-gnu-inline-cpp-without-extern -march=native -mtune=native -O3 -ffast-math")

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lm")

add_executable(nb_sc_decoder_abd ${CMAKE_CURRENT_SOURCE_DIR}/apps/nb_sc_decoder_abd.cpp)
target_include_directories(nb_sc_decoder_abd PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common)

#######################################################################################################################################

add_subdirectory (src)

#######################################################################################################################################

