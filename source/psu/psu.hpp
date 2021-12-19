/*******************************************************************************
 *  File:           PSU/psu.hpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   17 December, 2021
 * 
 *  Purpose:        This file houses the PSU object.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 ******************************************************************************/

#pragma once

#include <fmt/core.h>

#include <string>
#include <string_view>

#include <chrono>
#include <thread>

#include <CAENHVWrapper.h>

namespace msu_smdt
{
    struct com_port
    {
        std::string_view port;
        std::string_view baud_rate;
        std::string_view data_bit;
        std::string_view stop_bit;
        std::string_view parity;
        std::string_view lbusaddress;
    };

    class psu
    {
    public:
        // Here is our normal constructor. Notice that we are expecting a 
        // constant reference to the com_port structure. Hence, the structure
        // must exist in some capacity. The normal behavior is that the com_port
        // struct is first created, and then passed in.
        psu(const msu_smdt::com_port&);

        // Here are the move assignment operator and the move constructor. We
        // expect that these functions cannot throw exceptions, as that would
        // be terrible, and would indicate some sort of memory error.
        psu& operator=(psu&&) noexcept;
        psu(psu&&) noexcept;

        // Just our typical destructor.
        ~psu();

        // We do not want to be able to copy a psu object. This is because the
        // connection to the psu should be unique and exclusive. Therefore,
        // we cannot have two psu objects connected to the same psu.
        psu(const psu&) = delete;
        psu& operator=(const psu&) = delete;

        void print_internal_com_port_string();

    private:
        std::string_view com_port_str;
    };
}