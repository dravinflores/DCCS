cmake_minimum_required(VERSION 3.14)

project(check-development-environment CXX)

find_package(fmt REQUIRED)

add_executable(test_program source/main.cpp)
target_compile_features(test_program PRIVATE cxx_std_17)

target_link_libraries(test_program PRIVATE fmt::fmt)