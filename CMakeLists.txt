cmake_minimum_required(VERSION 3.14)

cmake_policy(SET CMP0091 NEW)
project(dccs CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(VCPKG_CRT_LINKAGE STATIC)

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