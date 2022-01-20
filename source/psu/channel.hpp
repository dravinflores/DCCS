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

    namespace status
    {
        constexpr uint_fast16_t ON      { 1 << 0 };
        constexpr uint_fast16_t RUP     { 1 << 1 };
        constexpr uint_fast16_t RDW     { 1 << 2 };
        constexpr uint_fast16_t OVC     { 1 << 3 };
        constexpr uint_fast16_t OVV     { 1 << 4 };
        constexpr uint_fast16_t UNV     { 1 << 5 };
        constexpr uint_fast16_t MAXV    { 1 << 6 };
        constexpr uint_fast16_t TRIP    { 1 << 7 };
        constexpr uint_fast16_t OVP     { 1 << 8 };
        constexpr uint_fast16_t OVT     { 1 << 9 };
        constexpr uint_fast16_t DIS     { 1 << 10 };
        constexpr uint_fast16_t KILL    { 1 << 11 };
        constexpr uint_fast16_t ILK     { 1 << 12 };
        constexpr uint_fast16_t NOLOCAL { 1 << 13 };
    }

    class channel
    {
    public:
        channel();
        channel(interchannel, int);
        channel(channel&&) noexcept;
        channel(const channel&);
        channel& operator=(channel&&) noexcept;
        channel& operator=(const channel&);
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
        uint_fast32_t get_status();

        void power_on();
        void power_off();

        void kill();

    private:
        interchannel info;
        std::string name;
        int channel_number;
        bool m_is_using_zero_current_adjust;
        double intrinsic_current;
        uint_fast32_t status;
    };
}