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

    enum class recover_mode
    {
        kill,
        ramp
    };

    class channel_manager
    {
    public:
        channel_manager();
        ~channel_manager();

        void initialize_channels(int, std::vector<unsigned short>);

        void enable_channel(unsigned short);
        void disable_channel(unsigned short);

        void set_programmed_voltage(unsigned short, float);
        void set_programmed_current_limit(unsigned short, float);
        void set_max_voltage_limit(unsigned short, float);
        void set_ramp_up_voltage_rate(unsigned short, float);
        void set_ramp_down_voltage_rate(unsigned short, float);
        void set_overcurrent_time_allowed(unsigned short, float);
        void set_method_of_powering_down(unsigned short, recover_mode);

        float read_voltage(unsigned short);
        bool is_in_high_precision_mode(unsigned short);
        float read_low_precision_current(unsigned short);
        float read_high_precision_current(unsigned short);
        bool is_normal_polarity(unsigned short);
        unsigned long read_channel_status(unsigned short);
        

    private:
        int handle;
        std::vector<channel> active_channels;
    };
}