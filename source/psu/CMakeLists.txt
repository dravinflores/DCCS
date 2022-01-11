# CMakeLists.txt

# Quick note on the CAEN HV Wrapper Library: the only files that actually
# need to see this library are contained in the source/psu/ folder. Therefore,
# only this target requires the use of the library, and we can localize 
# the finding of the library here.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
find_package(CAENHVWrapper REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/source/)

add_library(psu_object_library OBJECT psu.cpp psu.hpp channel.hpp)
add_library(ch_object_library OBJECT channel.cpp channel.hpp)
add_library(psuobj STATIC psu.cpp psu.hpp)

target_link_libraries(
    ch_object_library
    PRIVATE fmt::fmt
    PUBLIC CAENHVWrapper::CAENHVWrapper
)

target_link_libraries(
    psu_object_library
    PRIVATE fmt::fmt ch_object_library
    PUBLIC CAENHVWrapper::CAENHVWrapper
)

target_include_directories(
    psuobj
    PRIVATE fmt::fmt psu_object_library ch_object_library
)