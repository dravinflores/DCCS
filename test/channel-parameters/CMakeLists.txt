cmake_minimum_required(VERSION 3.14)

cmake_policy(SET CMP0091 NEW)
project(TestChannelParameters CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(CAENHVWrapper REQUIRED)

add_executable(test main.cpp)
target_compile_features(test PRIVATE cxx_std_17)
target_link_libraries(test PUBLIC CAENHVWrapper::CAENHVWrapper)
target_include_directories(test PUBLIC CAENHVWrapper::CAENHV_Wrapper)