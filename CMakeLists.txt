cmake_minimum_required(VERSION 3.20)

project(dccs VERSION 0.1.0 LANGUAGES C CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)
set(C_STANDARD                  11)

# A few variables that we can set based on how we want to compile everything.
option(VIRTUALIZE_HVLIB "Virtualize the connection to the CAEN HV Wrapper Library (HVLIB)" OFF)
option(TEST_FAKEHV "Check testing of the FakeHV Library" OFF)

if (MSVC)
    list(APPEND CMAKE_PREFIX_PATH D:/Qt/6.3.0/msvc2019_64)
elseif(APPLE)
    list(APPEND CMAKE_PREFIX_PATH $ENV{HOME}/Qt/6.3.0/macos)
endif()

set(SPDLOG_FMT_EXTERNAL ON)
set(BUILD_SHARED_LIBS OFF)

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

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz
)

FetchContent_MakeAvailable(fmt spdlog json)

find_package(
    Qt6
    REQUIRED COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

add_executable(dccs source/main.cpp)
add_subdirectory(source/psu)
add_subdirectory(source/gui)

target_include_directories(
    dccs 
    PRIVATE 
        source
)

target_link_libraries(
    dccs
    PRIVATE
        PSUController
        fmt::fmt
        spdlog::spdlog
        nlohmann_json::nlohmann_json
        Qt::Gui
        Qt::Core
        Qt::Widgets
)