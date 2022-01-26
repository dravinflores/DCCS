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

#include <span>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

#include <fmt/core.h>

#include <CAENHVWrapper.h>

template <typename T>
void set_channel_parameters(
    T val,
    int handle,
    unsigned short slot,
    std::string_view parameter_name,
    std::span<unsigned short> channels_to_set
)
{
    CAENHVRESULT result = CAENHV_SetChParam(
        handle,
        slot,
        parameter_name.data(),
        (unsigned short) channels_to_set.size(),
        (unsigned short*) channels_to_set.data(),
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


template <typename T>
std::vector<T> get_channel_parameters(
    T type_hint,
    int handle,
    unsigned short slot,
    std::string_view parameter_name,
    std::span<unsigned short> channels_to_get
)
{
    static constexpr int default_value { 256 };

    // We need to figure out how the library handles channels.
    // As in, can we pass in a list for just CH2 and CH3, and
    // hope that we only get a list of two vectors back?

    // Idea 1: If we only specify CH2 and CH3 in our list of 
    // channels, then only the first two elements of our array
    // will be populated.
    T returned_parameter_values[4] {
        (T) default_value,
        (T) default_value,
        (T) default_value,
        (T) default_value
    };

    // Idea 2: If we only specify CH2 and CH3, then we only need
    // to allocate two spaces for the return values.]
    std::unique_ptr<T[]> allocated_returned_parameter_values {
        new T[channels_to_get.size()]
    };

    for (int i = 0; i < channels_to_get.size(); ++i)
    {
        allocated_returned_parameter_values[i] = (T) default_value;
    }

    CAENHVRESULT result = CAENHV_GetChParam(
        handle,
        slot,
        parameter_name.data(),
        (unsigned short) channels_to_get.size(),
        (unsigned short*) channels_to_get.data(),
        returned_parameter_values
    );

    if (result != CAENHV_OK)
    {
        std::string error = fmt::format(
            "CAENHV_GetChParam() error: {}",
            CAENHV_GetError(handle) 
        );

        throw std::runtime_error(error.c_str());
    }

    std::vector<T> return_vec;

    for (int i = 0; i < channels_to_get.size(); ++i)
    {
        T val = returned_parameter_values[i];
        fmt::print("At {}: \n", i);
        fmt::print("\t{}: {}\n\n", parameter_name, val);
        return_vec.push_back(val);
    }

    return return_vec;
}