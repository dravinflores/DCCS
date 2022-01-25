/*******************************************************************************
 *  File:           psu/hv_channel_interface.hpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   25 January 2022
 * 
 *  Purpose:        This file provides a simple wrapper to two important
 *                  channel functions used in the CAEN HV Wrapper Library.
 *                  Rather than litter channel and psu source files with a
 *                  collection of otherwise-similar calls, we wrap these calls
 *                  into a higher level of abstraction.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <string_view>

#include <CAENHVWrapper.h>

template<typename T>
void set_channel_parameter(
    T val,
    int handle,
    std::string_view parameter_name,
    std::vector<uint_fast16_t> channels_to_set
)
{
    CAENHVRESULT result = CAENHV_SetChParam(
        /* handle */
        handle,

        /* slot */
        0,

        /* parameter name */
        parameter_name.data(),

        /* number of channels */
        (unsigned short) channels_to_set.size(),

        /* list of channels */
        (unsigned short*) channels_to_set.data(),

        /* parameter value */
        &val
    );

    if (result != CAENHV_OK)
    {
        std::string error = fmt::format(
            "CAENHV_SetChParam() error: {}",
            CAENHV_GetError(handle) 
        );

        throw std::runtime_error(error.c_str());
    } 
}


template<typename T>
std::vector<T> get_channel_parameter(
    T type_hint,
    int handle,
    std::string_view parameter_name,
    std::vector<uint_fast16_t> active_channel_list
)
{
    static constexpr int default_value { 256 };

    // In order to interface with the library, we are required to pass in
    // a dynamically allocated block of memory that can store an array of
    // parameter values. Rather than do this, we use std::vector's internal
    // book-keeping system, and just expose the pointer.
    std::vector<T> values_list { 
        default_value, 
        default_value, 
        default_value, 
        default_value
    };

    CAENHVRESULT result = CAENHV_GetChParam(
        /* handle */
        handle,

        /* slot */
        0,

        /* parameter name */
        parameter_name.data(),

        /* number of channels */
        (unsigned short) active_channel_list.size(),

        /* channel list */
        (unsigned short*) active_channel_list.data(),

        /* list of parameter values */
        (T*) values_list.data()
    );

    if (result != CAENHV_OK)
    {
        std::string error = fmt::format(
            "CAENHV_SetChParam() error: {}",
            CAENHV_GetError(handle) 
        );

        throw std::runtime_error(error.c_str());
    }
    
    return values_list;
}