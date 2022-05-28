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
#include <string>
#include <memory>

#include <spdlog/spdlog.h>

#include <psu/com_port_struct.hpp>

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

    class psu
    {
    public:
        // The default constructor. This is the only viable constructor.
        // A separate constructor is allowed for debugging purposes.
        psu(const msu_smdt::com_port& com_port_info);

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

        void start_test();

        void initialize_channels(std::vector<unsigned short> channels_to_activate);

        void enable_channel(unsigned short channel);
        void disable_channel(unsigned short channel);

        void set_programmed_voltage(unsigned short channel, float value);
        void set_programmed_current_limit(unsigned short channel, float value);
        void set_max_voltage_limit(unsigned short channel, float value);
        void set_ramp_up_voltage_rate(unsigned short channel, float value);
        void set_ramp_down_voltage_rate(unsigned short channel, float value);
        void set_overcurrent_time_allowed(unsigned short channel, float value);
        void set_method_of_powering_down(unsigned short channel, recover_mode mode);

        float read_voltage(unsigned short channel);
        bool is_in_high_precision_mode(unsigned short channel);
        float read_low_precision_current(unsigned short channel);
        float read_high_precision_current(unsigned short channel);
        bool is_normal_polarity(unsigned short channel);
        unsigned long read_channel_status(unsigned short channel);

    private:
        // Here are all the board specific pieces of information.
        std::string com_port_str;
        int handle;
        int number_of_channels;
        int serial_number;
        int slot;
        std::string firmware_version;

        // Here we will track the channels being activated and deactivated.
        std::vector<channel> active_channels;

        // The internal logger.
        std::shared_ptr<spdlog::logger> logger;
    };
}