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
void set_channel_parameter(
    T val,
    int handle,
    std::string_view parameter,
    unsigned short channel_number
)
{
#ifndef NDEBUG
    fmt::print("Function set_channel_parameter called.\n");
    fmt::print("Arguments passed in:\n");
    fmt::print("\tArgument 1: val = {}\n", val);
    fmt::print("\tArgument 2: handle = {}\n", handle);
    fmt::print("\tArgument 3: parameter = {}\n", parameter);
    fmt::print("\tArgument 4: channel_number = {}\n", channel_number);
#endif // NDEBUG

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
    {
        std::string error = fmt::format(
            "CAENHV_SetChParam() error: {}",
            CAENHV_GetError(handle) 
        );

    #ifndef NDEBUG
        fmt::print("Exception Caught! Printing then throwing.\n");
        fmt::print("\t{}\n", error);
    #endif // NDEBUG
        throw std::runtime_error(error.c_str());
    }
    else
    {
    #ifndef NDEBUG
        fmt::print("\n");
    #endif // NDEBUG
    }
}

template <typename T>
T get_channel_parameter(
    int handle,
    std::string_view parameter,
    unsigned short channel_number
)
{
#ifndef NDEBUG
    fmt::print("Function get_channel_parameter called.");
    fmt::print("Arguments passed in:\n");
    fmt::print("\tArgument 1: handle = {}\n", handle);
    fmt::print("\tArgument 2: parameter = {}\n", parameter);
    fmt::print("\tArgument 3: channel_number = {}\n", channel_number);
#endif // NDEBUG

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
    {
        std::string error = fmt::format(
            "CAENHV_SetChParam() error: {}",
            CAENHV_GetError(handle) 
        );

    #ifndef NDEBUG
        fmt::print("Exception Caught! Printing then throwing.\n");
        fmt::print("\t{}\n", error);
    #endif // NDEBUG
        throw std::runtime_error(error.c_str());
    }
    else
    {
    #ifndef NDEBUG
        fmt::print("\n");
    #endif // NDEBUG
    }

    return received_value;
}