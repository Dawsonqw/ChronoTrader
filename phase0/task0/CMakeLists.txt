add_compile_options(-O2)
add_executable(test_0 test_0.cpp)

# add_custom_target(test_0_asm 
#     COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS} -O2 -std=c++17 -S ${CMAKE_CURRENT_SOURCE_DIR}/test_0.cpp -o test_0.s
# )