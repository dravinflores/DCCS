# FindCAENHVWrapper.cmake

message(
    "\n"
    "Attempting to find the CAEN HV Wrapper Library.\n"
    "The location of the CAEN HV Wrapper library is assumed to be " 
    "C:/Program Files/CAEN/HV/CAENHVWrapper.\n"
    "To change this, refer to cmake/FindCAENHVWrapper.cmake\n"
)

include(FindPackageHandleStandardArgs)

find_library(
    CAENHVWrapper_LIBRARY
    NAMES CAENHVWrapper
    HINTS "C:/Program Files/CAEN/HV/CAENHVWrapper"
    PATH_SUFFIXES "lib/x86_64/"
    DOC "The CAEN HV Wrapper Library"
)

find_path(
    CAENHVWrapper_INCLUDE_DIR
    NAMES "CAENHVWrapper.h"
    HINTS "C:/Program Files/CAEN/HV/CAENHVWrapper"
    PATH_SUFFIXES "include/"
    DOC "The header file for the CAEN HV Wrapper Library"
)

find_package_handle_standard_args(
    CAENHVWrapper DEFAULT_MSG
    CAENHVWrapper_LIBRARY
    CAENHVWrapper_INCLUDE_DIR
)

mark_as_advanced(CAENHVWrapper_LIBRARY CAENHVWrapper_INCLUDE_DIR)

if(CAENHVWrapper_FOUND AND NOT TARGET CAENHVWraooer::CAENHVWrapper)
    add_library(CAENHVWrapper::CAENHVWrapper STATIC IMPORTED)
    set_target_properties(
        CAENHVWrapper::CAENHVWrapper
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${CAENHVWrapper_INCLUDE_DIR}"
            IMPORTED_LOCATION "${CAENHVWrapper_LIBRARY}"
    )
    target_include_directories(
        CAENHVWrapper::CAENHVWrapper INTERFACE "${CAENHVWrapper_INCLUDE_DIR}"
    )
endif()