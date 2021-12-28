# CMakeLists.txt

add_library(psuobj STATIC psu.cpp)
# target_include_directories(psuobj PRIVATE ./)

# Quick note on the CAEN HV Wrapper Library: the only files that actually
# need to see this library are contained in the source/psu/ folder. Therefore,
# only this target requires the use of the library, and we can localize 
# the finding of the library here.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
find_package(CAENHVWrapper REQUIRED)

target_link_libraries(psuobj PUBLIC CAENHVWrapper::CAENHVWrapper)

target_include_directories(
    psuobj 
    PRIVATE ./ 
    PUBLIC CAENHVWrapper::CAENHV_Wrapper
)

