/*******************************************************************************
 *  File:           psu/psu.hpp
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
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <stdexcept>

#include <psu/channel.hpp>

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
        // The default constructor. This is the only viable constructor.
        // A separate constructor is allowed for debugging purposes.
        psu();

        // We do not want to be able to move a psu object. This is because
        // we would need to reinitialize with the power supply upon the move.
        psu& operator=(psu&&) = delete;
        psu(psu&&) = delete;

        // We do not want to be able to copy a psu object. This is because the
        // connection to the psu should be unique and exclusive. Therefore,
        // we cannot have two psu objects connected to the same psu.
        psu(const psu&) = delete;
        psu& operator=(const psu&) = delete;

        // Just our typical destructor.
        ~psu();

        void initialize(const msu_smdt::com_port&);

        void start_test(int);

        void print_internal_com_port_string();

    private:
        // Here are all the board specific pieces of information.
        std::string com_port_str;
        int handle;
        int number_of_channels;
        int serial_number;
        int slot;
        std::string firmware_version;
        
        // Now we will have the channels here.
        channel_manager internal_manager;
    };
}