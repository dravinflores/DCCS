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

#include <vector>

#include <chrono>
#include <thread>

// We need to write this one out...
#include <cstring>

// This is for throwing runtime exceptions.
#include <stdexcept>

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

    enum class polarity
    {
        normal,
        reverse,
        simultaneous
    };

    class psu
    {
    public:
        // Here is our normal constructor. Notice that we are expecting a 
        // constant reference to the com_port structure. Hence, the structure
        // must exist in some capacity. The normal behavior is that the com_port
        // struct is first created, and then passed in.
        psu(const msu_smdt::com_port&);

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

        // The following are internal functions.
        void print_internal_com_port_string();

        int get_number_of_channels();
        void set_number_of_channels(int);

        polarity get_channel_polarity(int);

        void power_on_channel(int);
        void power_off_channel(int);
        void kill_all();
        void enable_all();
        void disable_all();

        double read_current(int);
        double read_voltage(int);

        void adjust_for_intrinsic_current();

        // For all intents and purposes, this is the main function that will
        // be called for a PSUOBJ. Internally, this function can all many
        // other functions.
        void start_test(int);

    private:
        std::string com_port_str;
        int handle;
        int number_of_channels;
        int serial_number;
        int slot;
        std::string firmware_version;
        std::vector<int> channel_list;
        std::vector<double> channel_intrinsic_currents;
    };
}