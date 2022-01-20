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
        ~channel();

        channel(const channel&);
        channel& operator=(const channel&);

        channel(channel&&) noexcept = delete;
        channel& operator=(channel&&) noexcept = delete;
        
        void initialize(const interchannel&, int);

        void set_operating_voltage(double);
        void set_current_limit(double);

        void set_voltage_limit(double);
        void set_overcurrent_time_limit(double);

        void set_voltage_increase_rate(double);
        void set_voltage_decrease_rate(double);

        double read_current();
        double read_voltage();
        polarity read_polarity();
        uint_fast32_t read_status();

        void interpret_status();

        void power_on();
        void power_off();
        void force_power_off();

    private:
        interchannel    info;
        int             channel_number;

        bool            is_using_zero_current_adjust;
        double          intrinsic_current;
    };
}