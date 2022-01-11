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

#include <CAENHVWrapper.h>

namespace msu_smdt
{
    enum class polarity
    {
        normal,
        reverse
    };

    struct interchannel
    {
        int handle;
        unsigned short slot;
        std::vector<unsigned short> channel_list;
    };

    class channel
    {
    public:
        channel();
        channel(channel&&) noexcept;
        channel(const channel&);
        channel& operator=(channel&&) noexcept;
        channel& operator=(const channel&);
        channel(interchannel, int);
        ~channel();

        void set_voltage(double);
        double read_voltage();

        void set_current(double);

        // If the current is between in the range of 0-2 uA, then
        // the current range will switch to low, and the readings
        // will be (+/-) 0.5 nA.
        double get_current();

        polarity get_polarity();

        void set_voltage_limit(double);
        void set_overcurrent_time_limit(double);

        void set_voltage_increase_rate(double);
        void set_voltage_decrease_rate(double);

        void adjust_for_zero_current();
        bool is_using_zero_current_adjust();

        void read_status();

        void power_on();
        void power_off();

        void kill();

    private:
        interchannel info;
        std::string name;
        int channel_number;
        bool m_is_using_zero_current_adjust;
        double intrinsic_current;
    };
}