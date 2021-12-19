cmake_minimum_required(VERSION 3.14)

project(dccs CXX)

find_package(fmt REQUIRED)


add_subdirectory(source/psu)

add_executable(dccs source/main.cpp)
target_compile_features(dccs PRIVATE cxx_std_17)
target_include_directories(dccs PRIVATE source)

target_link_libraries(psuobj PRIVATE fmt::fmt)
target_link_libraries(dccs PRIVATE fmt::fmt)
target_link_libraries(dccs PRIVATE psuobj)

set_target_properties(dccs PROPERTIES CXX_STANDARD 17)
set_target_properties(psuobj PROPERTIES CXX_STANDARD 17)