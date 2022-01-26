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
#include <memory>

#include <fmt/core.h>

#include <psu/hv_channel_interface.hpp>

#include <CAENHVWrapper.h>

namespace msu_smdt
{
    // This is more of an administrative abstraction, meant to 
    // help manage each of the channels more independently.
    struct channel
    {
        int number;
        bool is_active;
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
            int, 
            unsigned short, 
            std::vector<unsigned short>
        );

        void set_global_current_limit(double);
        void set_global_operating_voltage(double);

        void set_global_voltage_limit(double);
        void set_global_overcurrent_time_limit(double);

        void set_global_voltage_increase_rate(double);
        void set_global_voltage_decrease_rate(double);

        // Have to implement

        void set_single_channel_current_limit(unsigned short, double);
        void set_global_operating_voltage(unsigned short, double);

        void set_single_channel_voltage_limit(unsigned short, double);
        void set_single_channel_overcurrent_time_limit(unsigned short, double);

        void set_single_channel_voltage_increase_rate(unsigned short, double);
        void set_single_channel_voltage_decrease_rate(unsigned short, double);


        double read_channel_current(unsigned short);
        double read_channel_voltage(unsigned short);
        uint_fast32_t read_channel_status(unsigned short);
        uint_fast32_t read_channel_polarity(unsigned short);

        void interpret_status();

        void enable_channel(unsigned short);
        void disable_channel(unsigned short);
        void kill_channel(unsigned short);

        bool is_an_active_channel(unsigned short);
        std::vector<unsigned short> get_active_channel_numbers();

        // To implement later
        
        std::vector<double> read_multiple_currents(unsigned short);
        std::vector<double> read_multiple_voltages(unsigned short);
        std::vector<uint_fast32_t> read_multiple_statuses(unsigned short);
        std::vector<uint_fast32_t> read_multiple_polarities(unsigned short);

        void enable_multple_channels(unsigned short);
        void disable_multiple_channels(unsigned short);
        void kill_multiple_channel(unsigned short);

    private:
        int handle;
        unsigned short slot;
        std::vector<channel> active_channels;
    };
}