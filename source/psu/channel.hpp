/*******************************************************************************
 *  File:           psu/channel.hpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   03 January, 2022
 * 
 *  Purpose:        This file houses the class prototype for the channel.
 *                  This is so that the idea of a channel can be abstracted
 *                  away into a basic object with functionality.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 ******************************************************************************/

#pragma once

#include <map>
#include <tuple>
#include <string>
#include <vector>
#include <cstdint>
#include <utility>
#include <stdexcept>

#include <fmt/core.h>

#include <psu/hv_channel_interface.hpp>

#include <CAENHVWrapper.h>

namespace msu_smdt
{
    // This is more of an administrative abstraction, meant to 
    // help manage each of the channels more independently.
    struct channel
    {
        bool is_active;
        int number;
        double last_read_voltage;
        double last_read_current;
        double intrinsic_current;
    };

    class channel_manager
    {
    public:
        channel_manager();
        ~channel_manager();

        void initialize_channels(
            std::vector<uint_fast16_t>, 
            int, 
            uint_fast16_t
        );

        void set_global_operating_voltage(double);
        void set_global_current_limit(double);

        void set_global_voltage_limit(double);
        void set_global_overcurrent_time_limit(double);

        void set_global_voltage_increase_rate(double);
        void set_global_voltage_decrease_rate(double);

        double read_channel_current(int);
        double read_channel_voltage(int);
        uint_fast32_t read_channel_polarity(int);
        uint_fast32_t read_channel_status(int);

        void interpret_status();

        void enable_channel(int);
        void disable_channel(int);
        void kill_channel(int);

        bool is_an_active_channel(int);
        std::vector<uint_fast16_t> get_active_channel_numbers();

    private:
        std::vector<channel> active_channels;
        int handle;
        unsigned short slot;
    };
}