/***********************************************************************************************************************
 *  File:           psu/psu.cpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   17 December 2021
 * 
 *  Purpose:        This file defines all the functions prototyped in
 *                  the psu.hpp header file.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 **********************************************************************************************************************/

#include <chrono>
#include <thread>

#include <psu/psu.hpp>
#include <psu/hv_interface.hpp>

namespace msu_smdt
{
    static constexpr int default_value { 512 };

    psu::psu(const msu_smdt::com_port& com_port_info):
        com_port_str        { "DEFAULT" },
        handle              { -1 },
        number_of_channels  { -1 },
        serial_number       { -1 },
        slot                { -1 },
        firmware_version    { "DEFAULT" },
        active_channels     {
            // For CH0
            { 0, false, default_value, default_value, default_value },

            // For CH1
            { 1, false, default_value, default_value, default_value },

            // For CH2
            { 2, false, default_value, default_value, default_value },

            // For CH3
            { 3, false, default_value, default_value, default_value }
        },
        logger { spdlog::get("psu_logger") }
    {
        this->handle = initialize_system(com_port_info);

        std::tie(this->number_of_channels, this->serial_number, this->slot, this->firmware_version)
            = get_crate_map(this->handle);
    }

    psu::~psu()
    {
        deinitialize_system(this->handle);
    }

    void psu::start_test()
    {
        logger->debug("----- Initiating Test -----");
        logger->debug("Testing connection to CH0");
        enable_channel(0);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        auto voltage = read_voltage(0);
        auto current = read_low_precision_current(0);

        logger->debug("Voltage is: {}", voltage);
        logger->debug("Current is: {}", current);

        disable_channel(0);

        logger->debug("----- Test Completed -----");
    }

    void psu::initialize_channels(std::vector<unsigned short> channels_to_activate)
    {
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

    void psu::enable_channel(unsigned short channel)
    {
        unsigned long value = 1;
        set_channel_parameter(value, this->handle, "Pw", channel);
    }

    void psu::disable_channel(unsigned short channel)
    {
        unsigned long value = 0;
        set_channel_parameter(value, this->handle, "Pw", channel);  
    }

    void psu::set_programmed_voltage(unsigned short channel, float value)
    {
        set_channel_parameter(value, this->handle, "VSet", channel);
    }

    void psu::set_programmed_current_limit(unsigned short channel, float value)
    {
        set_channel_parameter(value, this->handle, "ISet", channel);
    }

    void psu::set_max_voltage_limit(unsigned short channel, float value)
    {
        set_channel_parameter(value, this->handle, "MaxV", channel);
    }

    void psu::set_ramp_up_voltage_rate(unsigned short channel, float value)
    {
        set_channel_parameter(value, this->handle, "RUp", channel);
    }

    void psu::set_ramp_down_voltage_rate(unsigned short channel, float value)
    {
        set_channel_parameter(value, this->handle, "RDwn", channel);
    }

    void psu::set_overcurrent_time_allowed(unsigned short channel, float value)
    {
        set_channel_parameter(value, this->handle, "Trip", channel);
    }

    void psu::set_method_of_powering_down(unsigned short channel, recover_mode mode)
    {
        unsigned long value = 1;

        if (mode == recover_mode::kill)
        {
            value = 0;
        }
        else
        {
            value = 1;
        }

        set_channel_parameter(value, this->handle, "PDwn", channel);
    }

    float psu::read_voltage(unsigned short channel)
    {
        auto voltage = get_channel_parameter<float>(this->handle, "VMon", channel);
        return voltage;
    }

    bool psu::is_in_high_precision_mode(unsigned short channel)
    {
        auto is_low = get_channel_parameter<unsigned long>(this->handle, "ImonRange", channel);
        return !is_low;
    }

    float psu::read_low_precision_current(unsigned short channel)
    {
        auto current = get_channel_parameter<float>(this->handle, "IMonL", channel);
        return current;
    }

    float psu::read_high_precision_current(unsigned short channel)
    {
        auto current = get_channel_parameter<float>(this->handle, "IMonH", channel);
        return current;
    }

    bool psu::is_normal_polarity(unsigned short channel)
    {
        auto is_reverse = get_channel_parameter<unsigned long>(this->handle, "Polarity", channel);
        return !is_reverse;
    }

    unsigned long psu::read_channel_status(unsigned short channel)
    {
        auto ch_status = get_channel_parameter<unsigned long>(this->handle, "ChStatus", channel);
        return ch_status;
    }
}