cmake_minimum_required(VERSION 3.14)

project(dccs CXX)

find_package(fmt REQUIRED)

add_executable(dccs source/main.cpp)
target_compile_features(dccs PRIVATE cxx_std_17)

target_link_libraries(dccs PRIVATE fmt::fmt)