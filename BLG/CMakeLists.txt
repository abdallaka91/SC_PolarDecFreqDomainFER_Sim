cmake_minimum_required(VERSION 3.21)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)

set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)

project(NB_SC_DECODER_EXPLORER LANGUAGES CXX)

option (SANITIZE_DECODERS "Add sanitization values" OFF)

if (SANITIZE_DECODERS)
	set (SANITIZE_CXXFLAGS "-fsanitize=address;-fsanitize=undefined;-fno-omit-frame-pointer;-ggdb")
	set (SANITIZE_LDFLAGS "-fsanitize=address;-fsanitize=undefined;-fno-omit-frame-pointer;-ggdb")
endif ()

#SET (CMAKE_C_COMPILER   /opt/homebrew/opt/llvm/bin/clang)     # LLVM 21.1.7
#SET (CMAKE_CXX_COMPILER /opt/homebrew/opt/llvm/bin/clang++)   # LLVM 21.1.7

#SET (CMAKE_C_COMPILER   /opt/homebrew/Cellar/llvm@20/20.1.8/bin/clang)     # LLVM 20.1.8
#SET (CMAKE_CXX_COMPILER /opt/homebrew/Cellar/llvm@20/20.1.8/bin/clang++)   # LLVM 20.1.8

#SET (CMAKE_C_COMPILER   /opt/homebrew/Cellar/llvm@18/18.1.8/bin/clang)     # LLVM 18.1.8
#SET (CMAKE_CXX_COMPILER /opt/homebrew/Cellar/llvm@18/18.1.8/bin/clang++)   # LLVM 18.1.8

#SET (CMAKE_C_COMPILER   /opt/homebrew/bin/gcc-13)     # GCC 13.4.0
#SET (CMAKE_CXX_COMPILER /opt/homebrew/bin/g++-13)   # GCC 13.4.0

#SET (CMAKE_C_COMPILER   /opt/homebrew/bin/gcc-15)     # GCC 15.2.0
#SET (CMAKE_CXX_COMPILER /opt/homebrew/bin/g++-15)   # GCC 15.2.0


add_compile_options (-march=native -mtune=native -O3 -g0 ${SANITIZE_CXXFLAGS})
# add_compile_options(-g -O0 -march=native -mtune=native -Wall -Wextra ${SANITIZE_CXXFLAGS})#debug2
add_compile_options (-Wall -Wextra)
# to remove warnings from ap_int
add_compile_options (-Wno-unused-parameter -Wno-deprecated-copy -Wno-int-in-bool-context -Wno-type-limits -Wno-strict-aliasing -Wno-extra)
add_link_options (${SANITIZE_LDFLAGS})

#######################################################################################################################################

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-gnu-inline-cpp-without-extern -march=native -mtune=native -O3 -ffast-math -g3")

#add_executable (nb_sc_decoder   ${CMAKE_CURRENT_SOURCE_DIR}/apps/nb_sc_decoder.cpp
#        src/decoders/specialized_pruning/decoder_specialized_pruning.cpp
#        src/decoders/specialized/decoder_specialized.cpp
#        src/decoders/naive/decoder_naive.cpp
#        src/decoders/naive_pruning/decoder_naive_pruning.cpp
#)
#target_include_directories (nb_sc_decoder PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common)
#target_link_libraries (nb_sc_decoder PUBLIC utilities)

#######################################################################################################################################

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-gnu-inline-cpp-without-extern -march=native -mtune=native -O3 -ffast-math -g3")

add_executable(benchmarking
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/benchmarking.cpp
        src/decoders/specialized_pruning/decoder_specialized_pruning.cpp
        src/decoders/specialized/decoder_specialized.cpp
        src/decoders/dedicated/decoder_dedicated.cpp
		src/decoders/naive/decoder_naive.cpp
        src/decoders/basic/decoder_basic.cpp
        src/decoders/naive_fixed/decoder_naive_fixed.cpp
		src/decoders/naive_int32_t/decoder_naive_int32_t.cpp
		src/decoders/naive_cfloat/decoder_naive_cfloat.cpp
		src/hadamard/Hadamard.cpp
        src/encoder/polar_encoder.cpp
)
target_include_directories (benchmarking PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common)
target_link_libraries      (benchmarking PUBLIC utilities)

#######################################################################################################################################

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-gnu-inline-cpp-without-extern -march=native -mtune=native -O3 -ffast-math -g3")

add_executable(fwht_validator
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/bench_fwht.cpp
)
target_include_directories (fwht_validator PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common)
target_link_libraries      (fwht_validator PUBLIC utilities)

#######################################################################################################################################

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas -Wno-gnu-inline-cpp-without-extern -march=native -mtune=native -O3 -ffast-math -g3")

add_executable(code_generator
        ${CMAKE_CURRENT_SOURCE_DIR}/apps/code_generator.cpp
        src/encoder/polar_encoder.cpp
)
target_include_directories (code_generator PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common)
target_link_libraries      (code_generator PUBLIC utilities)

#######################################################################################################################################

add_subdirectory (src)

#######################################################################################################################################

add_library(NbScFFTdec SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/apps/LibNbScFFTdec.cpp
	src/decoders/specialized_pruning/decoder_specialized_pruning.cpp
	src/decoders/specialized/decoder_specialized.cpp
	src/decoders/dedicated/decoder_dedicated.cpp
	src/decoders/naive/decoder_naive.cpp
	src/decoders/basic/decoder_basic.cpp
	src/decoders/naive_fixed/decoder_naive_fixed.cpp
	src/decoders/naive_int32_t/decoder_naive_int32_t.cpp
	src/decoders/naive_cfloat/decoder_naive_cfloat.cpp
	src/hadamard/Hadamard.cpp
    src/encoder/polar_encoder.cpp
)
target_compile_options(NbScFFTdec PUBLIC "$<$<CONFIG:Debug>:${GCC_COMPILE_DEBUG_OPTIONS}>" -DALL_GFs)
target_compile_options(NbScFFTdec PUBLIC "$<$<CONFIG:Release>:${GCC_COMPILE_RELEASE_OPTIONS}>" -DALL_GFs)
#add_compile_options (-DALL_GFs)
target_include_directories (NbScFFTdec PUBLIC ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/common)
target_link_libraries      (NbScFFTdec PUBLIC utilities)

#######################################################################################################################################
