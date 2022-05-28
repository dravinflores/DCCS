/***********************************************************************************************************************
 *  File:           psu/com_port_struct.hpp
 *  Author:         Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   27 May 2022
 * 
 *  Purpose:        This header simply defines the com_port struct, which 
 *                  encapsulates all the information required for the passing
 *                  of com_port information.
 * 
 *  Known Issues:
 * 
 *  Workarounds:
 * 
 *  Updates:
 **********************************************************************************************************************/

#pragma once

#include <string_view>

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
}