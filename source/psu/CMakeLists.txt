include_directories(${CMAKE_SOURCE_DIR}/source/)

add_library(
    HVInterface
    OBJECT
        HVInterface.cpp
        HVInterface.hpp
        Port.hpp
)

target_link_libraries(
    HVInterface
    PRIVATE
        fmt::fmt
        spdlog::spdlog
)

if (VIRTUALIZE_HVLIB)
    add_compile_definitions(VIRTUALIZE_CONNECTION)
    add_subdirectory(FakeHV)
    target_link_libraries(
        HVInterface
        PRIVATE
            FakeHV
    )
else()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
    find_package(CAENHVWrapper REQUIRED)

    target_link_libraries(
        HVInterface
        PUBLIC
            CAENHVWrapper::CAENHVWrapper
    )

    target_include_directories(
        HVInterface
        PUBLIC
            CAENHVWrapper::CAENHVWrapper
    )
endif()

add_library(
    PSUController
    STATIC
        PSUController.cpp
        PSUController.hpp
)

target_link_libraries(
    PSUController
    PRIVATE
        HVInterface
        fmt::fmt
        spdlog::spdlog
)