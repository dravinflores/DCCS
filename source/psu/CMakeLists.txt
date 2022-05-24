# CMakeLists.txt

# Quick note on the CAEN HV Wrapper Library: the only files that actually
# need to see this library are contained in the source/psu/ folder. Therefore,
# only this target requires the use of the library, and we can localize 
# the finding of the library here.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
find_package(CAENHVWrapper REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/source/)

# Earlier attempts to create this object attempted to create a 
# channel object and a psu object, and then link them together
# into a singular psuobj. This failed spectacularly, for no
# obvious reason. This method is much simpler.
add_library(
    psuobj 
    OBJECT 
        psu.cpp psu.hpp channel.cpp channel.hpp hv_channel_interface.hpp
)

target_include_directories(
    psuobj
    PUBLIC
        CAENHVWrapper::CAENHVWrapper   
    PRIVATE 
        fmt::fmt
)

target_link_libraries(
    psuobj
    PUBLIC
        CAENHVWrapper::CAENHVWrapper   
    PRIVATE 
        fmt::fmt
)