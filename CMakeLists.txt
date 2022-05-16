cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0091 NEW)
project(dccs CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(VCPKG_CRT_LINKAGE STATIC)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)

add_subdirectory(source/psu)
add_executable(dccs source/main.cpp)

target_include_directories(
    dccs 
    PRIVATE 
        source
)

target_link_libraries(
    dccs 
    PUBLIC 
        CAENHVWrapper::CAENHVWrapper
    PRIVATE
        fmt::fmt
        spdlog::spdlog
        tomlplusplus::tomlplusplus
        psuobj
)