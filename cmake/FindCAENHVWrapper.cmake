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
    PATHS "C:/Program Files/CAEN/HV/CAENHVWrapper"
    PATH_SUFFIXES "lib/x86_64/"
    DOC "The CAEN HV Wrapper Library"
    REQUIRED
)

find_path(
    CAENHVWrapper_INCLUDE_DIR
    NAMES "CAENHVWrapper.h"
    PATHS "C:/Program Files/CAEN/HV/CAENHVWrapper"
    PATH_SUFFIXES "include/"
    DOC "The header file for the CAEN HV Wrapper Library"
    REQUIRED
)

find_package_handle_standard_args(
    CAENHVWrapper 
    DEFAULT_MSG
    CAENHVWrapper_LIBRARY
    CAENHVWrapper_INCLUDE_DIR
)

mark_as_advanced(CAENHVWrapper_LIBRARY CAENHVWrapper_INCLUDE_DIR)

if(CAENHVWrapper_FOUND AND NOT TARGET CAENHVWrapper::CAENHVWrapper)
    add_library(CAENHVWrapper::CAENHVWrapper SHARED IMPORTED GLOBAL)
    
    set_target_properties(
        CAENHVWrapper::CAENHVWrapper
        PROPERTIES
            IMPORTED_LOCATION "C:/Windows/System32"
            IMPORTED_IMPLIB "${CAENHVWrapper_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${CAENHVWrapper_INCLUDE_DIR}"
    )
endif()