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

#include <psu/psu.hpp>

#include <chrono>
#include <thread>

static constexpr int default_value { 512 };

read_only_psu::read_only_psu():
    logger { spdlog::get("hvlib_logger") }
{}

read_only_psu::~read_only_psu()
{}

float read_only_psu::read_voltage(unsigned short channel)
{
    return interface.get_parameter_float("VMon", channel);
}

bool read_only_psu::is_in_high_precision_mode(unsigned short channel)
{
    return !(interface.get_parameter_long("ImonRange", channel));
}

float read_only_psu::read_low_precision_current(unsigned short channel)
{
    return interface.get_parameter_float("IMonL", channel);
}

float read_only_psu::read_high_precision_current(unsigned short channel)
{
    return interface.get_parameter_float("IMonH", channel);
}

bool read_only_psu::is_normal_polarity(unsigned short channel)
{
    return !(interface.get_parameter_long("Polarity", channel));
}

unsigned long read_only_psu::read_channel_status(unsigned short channel)
{
    return interface.get_parameter_long("ChStatus", channel);
}

//

control_psu::control_psu(const msu_smdt::com_port& port_info):
    has_force_disconnected { false }
{
    interface.connect(port_info);
}

control_psu::~control_psu()
{
    if (!has_force_disconnected)
    {
        interface.disconnect();
    }
}

void control_psu::pretend_start()
{
    logger->debug("----- Initiating Test -----");
    logger->debug("Testing connection to CH0");
    power_channel_on(0);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    auto voltage = read_voltage(0);
    auto current = read_low_precision_current(0);

    logger->debug("Voltage is: {}", voltage);
    logger->debug("Current is: {}", current);

    power_channel_off(0);

    logger->debug("----- Test Completed -----");
}

void control_psu::force_disconnect()
{
    interface.disconnect();
    has_force_disconnected = true;
}

void control_psu::initialize_channels(std::vector<unsigned short> channels_to_activate)
{
    for (auto& channel : channels_to_activate)
    {
        set_programmed_voltage(channel, 15.00f);
        set_programmed_current_limit(channel, 2.00f);
        set_max_voltage_limit(channel, 4015.00f);
        set_ramp_up_voltage_rate(channel, 15.00f);
        set_ramp_down_voltage_rate(channel, 15.00f);
        set_overcurrent_time_allowed(channel, 1000.00f);
        set_method_of_powering_down(channel, recovery_mode::ramp);
    }
}

void control_psu::power_channel_on(unsigned short channel)
{
    unsigned long value = 1;
    interface.set_parameter_long(value, "Pw", channel);
}

void control_psu::power_channel_off(unsigned short channel)
{
    unsigned long value = 0;
    interface.set_parameter_long(value, "Pw", channel);  
}

void control_psu::set_programmed_voltage(unsigned short channel, float value)
{
    interface.set_parameter_float(value, "VSet", channel);
}

void control_psu::set_programmed_current_limit(unsigned short channel, float value)
{
    interface.set_parameter_float(value, "ISet", channel);
}

void control_psu::set_max_voltage_limit(unsigned short channel, float value)
{
    interface.set_parameter_float(value, "MaxV", channel);
}

void control_psu::set_ramp_up_voltage_rate(unsigned short channel, float value)
{
    interface.set_parameter_float(value, "RUp", channel);
}

void control_psu::set_ramp_down_voltage_rate(unsigned short channel, float value)
{
    interface.set_parameter_float(value, "RDwn", channel);
}

void control_psu::set_overcurrent_time_allowed(unsigned short channel, float value)
{
    interface.set_parameter_float(value, "Trip", channel);
}

void control_psu::set_method_of_powering_down(unsigned short channel, recovery_mode mode)
{
    unsigned long value = 1;

    if (mode == recovery_mode::kill)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    interface.set_parameter_long(value, "PDwn", channel);
}
