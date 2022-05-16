/***********************************************************************************************************************
 *  File:           psu/channel.cpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   03 January, 2022
 * 
 *  Purpose:        This file houses the the defitions of all the class methods 
 *                  for the channel class.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 **********************************************************************************************************************/

#include <psu/channel.hpp>

namespace msu_smdt
{ 
    static constexpr int default_value { 512 };

    channel_manager::channel_manager():
        handle  { -1 },
        active_channels {
            // For CH0
            { 0, false, default_value, default_value, default_value },

            // For CH1
            { 1, false, default_value, default_value, default_value },

            // For CH2
            { 2, false, default_value, default_value, default_value },

            // For CH3
            { 3, false, default_value, default_value, default_value },
        }
    {}

    channel_manager::~channel_manager()
    {}


    void channel_manager::initialize_channels(int handle, std::vector<unsigned short> channels_to_activate)
    {
        this->handle = handle;

        for (auto& channel : channels_to_activate)
        {
            this->active_channels[channel].is_active = true;

            set_programmed_voltage(channel, 15.00F);
            set_programmed_current_limit(channel, 2.00F);
            set_max_voltage_limit(channel, 4015.00F);
            set_ramp_up_voltage_rate(channel, 15.00F);
            set_ramp_down_voltage_rate(channel, 15.00F);
            set_overcurrent_time_allowed(channel, 1000.00F);
            set_method_of_powering_down(channel, recover_mode::ramp);
        }
    }

    void channel_manager::enable_channel(unsigned short channel)
    {
        unsigned long value = 1;

        set_channel_parameter(value, this->handle, "Pw", channel);
    }

    void channel_manager::disable_channel(unsigned short channel)
    {
        unsigned long value = 0;

        set_channel_parameter(value, this->handle, "Pw", channel);   
    }


    void channel_manager::set_programmed_voltage(unsigned short channel, float val)
    {
        set_channel_parameter(val, this->handle, "VSet", channel);
    }

    void channel_manager::set_programmed_current_limit(unsigned short channel, float val)
    {
        set_channel_parameter(val, this->handle, "ISet", channel);
    }

    void channel_manager::set_max_voltage_limit(unsigned short channel, float val)
    {
        set_channel_parameter(val, this->handle, "MaxV", channel);
    }

    void channel_manager::set_ramp_up_voltage_rate(unsigned short channel, float val)
    {
        set_channel_parameter(val, this->handle, "RUp", channel);
    }

    void channel_manager::set_ramp_down_voltage_rate(unsigned short channel, float val)
    {
        set_channel_parameter(val, this->handle, "RDwn", channel);
    }

    void channel_manager::set_overcurrent_time_allowed(unsigned short channel, float val)
    {
        set_channel_parameter(val, this->handle, "Trip", channel);
    }


    void channel_manager::set_method_of_powering_down(unsigned short channel, recover_mode mode)
    {
        unsigned long val = 1;

        if (mode == recover_mode::kill)
        {
            val = 0;
        }
        else
        {
            val = 1;
        }

        set_channel_parameter(val, this->handle, "PDwn", channel);
    }


    float channel_manager::read_voltage(unsigned short channel)
    {
        auto voltage = get_channel_parameter<float>(this->handle, "VMon", channel);

        return voltage;
    }

    bool channel_manager::is_in_high_precision_mode(unsigned short channel)
    {  
        auto is_low = get_channel_parameter<float>(this->handle, "ImonRange", channel);

        return !is_low;
    }

    float channel_manager::read_low_precision_current(unsigned short channel)
    {
        auto current = get_channel_parameter<float>(this->handle, "IMonL", channel);

        return current;
    }

    float channel_manager::read_high_precision_current(unsigned short channel)
    {
        auto current = get_channel_parameter<float>(this->handle, "IMonH", channel);

        return current;
    }

    bool channel_manager::is_normal_polarity(unsigned short channel)
    {
        auto is_reverse = get_channel_parameter<float>(this->handle, "Polarity", channel);

        return !is_reverse;
    }

    unsigned long channel_manager::read_channel_status(unsigned short channel)
    {
        auto ch_status = get_channel_parameter<float>(this->handle, "ChStatus", channel);

        return ch_status;
    }
}