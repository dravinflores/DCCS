/*******************************************************************************
 *  File:           psu/hv_channel_interface.hpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   25 January 2022
 * 
 *  Purpose:        This file provides a simple interface to the CAEN HV
 *                  Wrapper Library's getter and setter functions for the
 *                  channel parameters; in particular CAENHV_GetChParam and 
 *                  CAENHV_SetChParam.
 * 
 *                  These functions operate on a multi-channel basis, which is
 *                  why a std::vector is returned.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 ******************************************************************************/

#pragma once

#include <memory>
#include <string>
#include <string_view>

#include <fmt/core.h>

#include <CAENHVWrapper.h>


// We are having serious problems trying to interface with the power supply.
// In order to try to appease this, we will restrict the setting and getting
// of channel parameters to a single channel.

template <typename T>
void set_channel_parameter(T val, int handle, std::string_view parameter, unsigned short channel_number)
{
    unsigned short slot = 0;
    const char* parameter_name_casted = (const char*) parameter.data();
    unsigned short number_of_channels = 1;
    T* address_of_parameter = &val;

    // Does this work, or am I going to have to do a memory allocation?
    unsigned short channel_list[1] = { channel_number };

    CAENHVRESULT result = CAENHV_SetChParam(
        handle,
        slot,
        parameter_name_casted,
        number_of_channels,
        channel_list,
        (void*) address_of_parameter
	);

    if (result != CAENHV_OK)
    {}
    else
    {}
}

template <typename T>
T get_channel_parameter(int handle, std::string_view parameter, unsigned short channel_number)
{
    unsigned short slot = 0;
    const char* parameter_name_casted = (const char*) parameter.data();
    unsigned short number_of_channels = 1;
    unsigned short channel_list[1] = { channel_number };

    // Let's see if we can avoid the dynamic memory allocation.
    T received_value = (T) 256;

    CAENHVRESULT result = CAENHV_GetChParam(
        handle,
        slot,
        parameter_name_casted,
        number_of_channels,
        channel_list,
        (void*) &received_value
	);

    if (result != CAENHV_OK)
    {}
    else
    {}

    return received_value;
}