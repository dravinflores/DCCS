/***********************************************************************************************************************
 *  File:           psu/psu.hpp
 *  Author:         Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   17 December 2021
 * 
 *  Purpose:        This file houses the psu object, which is the primary
 *                  method of interfacing with the power supply.
 * 
 *  Known Issues:
 * 
 *  Updates:
 **********************************************************************************************************************/

#pragma once

#include <vector>

#include <spdlog/spdlog.h>

#include <psu/com_port_struct.hpp>
#include <psu/hv_interface.hpp>

enum class recovery_mode
{
    kill,
    ramp
};

class read_only_psu
{
public:
    read_only_psu();
    ~read_only_psu();

    read_only_psu(read_only_psu&&) = delete;
    read_only_psu(const read_only_psu&) = delete;
    read_only_psu& operator=(read_only_psu&&) = delete;
    read_only_psu& operator=(const read_only_psu&) = delete;
    
    float read_voltage(unsigned short channel);
    bool is_in_high_precision_mode(unsigned short channel);
    float read_low_precision_current(unsigned short channel);
    float read_high_precision_current(unsigned short channel);
    bool is_normal_polarity(unsigned short channel);
    unsigned long read_channel_status(unsigned short channel);

protected:
    std::shared_ptr<spdlog::logger> logger;

private:
    hv_interface interface;
};


class control_psu : private read_only_psu
{
public:
    control_psu(const msu_smdt::com_port& port_info);
    ~control_psu();

    control_psu(control_psu&&) = delete;
    control_psu(const control_psu&) = delete;
    control_psu& operator=(control_psu&&) = delete;
    control_psu& operator=(const control_psu&) = delete;

    void pretend_start();

    void force_disconnect();

    void initialize_channels(std::vector<unsigned short> channels_to_activate);

    void power_channel_on(unsigned short channel);
    void power_channel_off(unsigned short channel);

    void set_programmed_voltage(unsigned short channel, float value);
    void set_programmed_current_limit(unsigned short channel, float value);
    void set_max_voltage_limit(unsigned short channel, float value);
    void set_ramp_up_voltage_rate(unsigned short channel, float value);
    void set_ramp_down_voltage_rate(unsigned short channel, float value);
    void set_overcurrent_time_allowed(unsigned short channel, float value);
    void set_method_of_powering_down(unsigned short channel, recovery_mode mode);

private:
    hv_interface interface;
    bool has_force_disconnected;
};