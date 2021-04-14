cmake_minimum_required(VERSION 3.19)
project(keepit)

set(CMAKE_CXX_STANDARD 17)

find_package (Threads)

add_executable(keepit main.cpp words_counter.cpp thread_pool.cpp)
target_compile_options(keepit PRIVATE -fno-exceptions)

target_link_libraries (keepit ${CMAKE_THREAD_LIBS_INIT})