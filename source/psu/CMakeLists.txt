include_directories(${CMAKE_SOURCE_DIR}/source/)

add_library(
    hv_interface
    OBJECT
        hv_interface.hpp
        com_port_struct.hpp
)

target_link_libraries(
    hv_interface
    PRIVATE
        fmt::fmt
        spdlog::spdlog
)

if (VIRTUALIZE_HVLIB)
    add_compile_definitions(VIRTUALIZE_CONNECTION)
else()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
    find_package(CAENHVWrapper REQUIRED)

    target_link_libraries(
        hv_interface
        PUBLIC
            CAENHVWrapper::CAENHVWrapper
    )

    target_include_directories(
        hv_interface
        PUBLIC
            CAENHVWrapper::CAENHVWrapper
    )
endif()

add_library(
    psu
    STATIC
        psu.cpp
        psu.hpp
)

target_link_libraries(
    psu
    PRIVATE
        hv_interface
        fmt::fmt
        spdlog::spdlog
)