cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0091 NEW)
project(dccs CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Pull any needed libraries before adding subdirectories, so that each of the 
# subdirectories can use the libraries.

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG master
)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.x
)

FetchContent_MakeAvailable(fmt spdlog)

option(SPDLOG_FMT_EXTERNAL "Using an external fmt library for spdlog." ON)

list(APPEND CMAKE_PREFIX_PATH D:/Qt/6.3.0/msvc2019_64)
find_package(
    Qt6
    COMPONENTS REQUIRED
        SerialPort
)
qt_standard_project_setup()

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
        Qt6::SerialPort
        spdlog::spdlog
        fmt::fmt
        psuobj
)