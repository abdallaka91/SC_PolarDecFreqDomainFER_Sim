
add_library (utilities utilities/utility_functions.cpp)
target_include_directories (utilities PUBLIC ${CMAKE_SOURCE_DIR}/src)

# -Wno-unknown-pragmas -Wno-gnu-inline-cpp-without-extern -march=native -mtune=native -O3

add_executable (bench_fwht ../apps/bench_fwht.cpp)
target_include_directories (bench_fwht PUBLIC ${CMAKE_SOURCE_DIR}/src)

#add_executable (test_function_f ${CMAKE_CURRENT_SOURCE_DIR}/node_functions/test_function_f.cpp)
#target_include_directories (test_function_f PUBLIC ${CMAKE_SOURCE_DIR}/src)
#target_link_libraries (test_function_f PUBLIC utilities)

