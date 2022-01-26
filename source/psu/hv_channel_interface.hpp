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
#include <vector>
#include <string>
#include <string_view>

#include <fmt/core.h>

#include <CAENHVWrapper.h>

// The proper use case for this function is as follows: create a vector
// of the particular channels whose properties are to be set. For instance,
// to set ONLY a parameter on CH3, the vector would be:
//      std::vector<unsigned short> channels_to_set { 3 };
template <typename T>
void set_channel_parameters(
    T val,
    int handle,
    std::string_view parameter_name,
    std::vector<unsigned short> channels_to_set
)
{
    unsigned short n = (unsigned short) channels_to_set.size();
    std::unique_ptr<unsigned short[]> channel_list { new unsigned short[n] };

    for (int i = 0; i < n; ++i)
    {
        channel_list[i] = channels_to_set.at(i);
    }

    CAENHVRESULT result = CAENHV_SetChParam(
        handle,
        (unsigned short) 0,
        parameter_name.data(),
        n,
        channel_list.get(),
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


// Similar use case to the above. Only, a vector will be returned which
// corresponds element-by-element to the channels being passed in.
template <typename T>
std::vector<T> get_channel_parameters(
    T type_hint,
    int handle,
    std::string_view parameter_name,
    std::vector<unsigned short> channels_to_get
)
{
    static constexpr int default_value { 256 };

    unsigned short n = channels_to_get.size();
    std::unique_ptr<unsigned short[]> channel_list { new unsigned short[n] };

    for (int i = 0; i < n; ++i)
    {
        channel_list[i] = channels_to_get.at(i);
    }

    std::unique_ptr<T[]> parameter_value_list { new T[n] };
    for (int i = 0; i < n; ++i)
    {
        parameter_value_list[i] = (T) default_value;
    }

    CAENHVRESULT result = CAENHV_GetChParam(
        handle,
        (unsigned short) 0,
        parameter_name.data(),
        n,
        channel_list.get(),
        parameter_value_list.get()
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
    
    for (int i = 0; i < n; ++i)
    {
        T recorded_val = (T) parameter_value_list[i];
        return_vec.push_back(recorded_val);
    }

    return return_vec;
}