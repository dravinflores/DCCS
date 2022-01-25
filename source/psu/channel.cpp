/*******************************************************************************
 *  File:           psu/channel.cpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   03 January, 2022
 * 
 *  Purpose:        This file houses the the defitions of all the class
 *                  methods for the channel class.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 ******************************************************************************/

/*
 *  Parameters as obtained from the PSU using CAENHV_GetChParamProp()
 *      VSet: 
 *          - Type: Float
 *          - Unit: V
 * 
 *      VMon: 
 *          - Type: Float
 *          - Unit: V
 * 
 *      ISet:
 *          - Type: Float
 *          - Unit: uA
 * 
 *      ImonRange:
 *          - Type: Boolean (as an Unsigned Int)
 *          - 0: High
 *          - 1: Low
 * 
 *      IMonL:
 *          - Type: Float
 *          - Unit: uA
 * 
 *      IMonH:
 *          - Type: Float
 *          - Unit: uA
 * 
 *      MaxV:
 *          - Type: Float
 *          - Unit: V
 * 
 *      RUp:
 *          - Type: Float
 *          - Unit: V/s
 * 
 *      RDwn:
 *          - Type: Float
 *          - Unit: V/s
 * 
 *      Trip:
 *          - Type: Float
 *          - Unit: s
 * 
 *      PDwn:
 *          - Type: Boolean (as an Unsigned Int)
 *          - 0: Kill
 *          - 1: Ramp
 * 
 *      Polarity:
 *          - Type: Boolean (as an Unsigned Int)
 *          - 0: +
 *          - 1: -
 * 
 *      ChStatus: 
 *          - Type: Bit Field
 * 
 *      Pw:
 *          - Type: Boolean (as an Unsigned Int)
 *          - 0: Off
 *          - 1: On
 */

#include <psu/channel.hpp>

namespace msu_smdt
{
    static constexpr int default_value { 256 };

    channel_manager::channel_manager():
        active_channels {
            { false, 0, default_value, default_value, default_value },
            { false, 1, default_value, default_value, default_value },
            { false, 2, default_value, default_value, default_value },
            { false, 3, default_value, default_value, default_value }
        },
        handle          { default_value },
        slot            { default_value }
    {
    #ifndef NDEBUG
        fmt::print("Constructor has been called on channel manager\n");
    #endif // NDEBUG
    }

    channel_manager::~channel_manager()
    {
    #ifndef NDEBUG
        fmt::print("Destructor has been called on channel manager\n");
    #endif // NDEBUG
    }


    void channel_manager::initialize_channels(
        std::vector<uint_fast16_t> channels_to_activate,
        int handle,
        uint_fast16_t slot
    )
    {
        // We expect that the user will pass in the particular
        // channels to activate. For instance, if the user wants
        // CH2 and CH3 active, then the passed in vector will be
        // std::vector<uint_fast16_t> channels { 2, 3 };

        // We want to ensure that we only activate the proper channels.
        for (auto& channel_number : channels_to_activate)
        {
            this->active_channels.at(channel_number).is_active = true;
        }

        this->handle = handle;
        this->slot = slot;

        // This enumeration is useless outside of this function.
        enum class Type
        {
            Float,
            Int,
            Unknown
        };

        // Now we want to initialize all channels to default values.
        std::vector<std::tuple<std::string, double, Type>> default_parameters {
            { "VSet",   3015.00,    Type::Float },
            { "ISet",   2.00,       Type::Float },
            { "MaxV",   4015.00,    Type::Float },
            { "RUp",    500.00,     Type::Float },
            { "RDwn",   500.00,     Type::Float },
            { "Trip",   1000.00,    Type::Float },
            { "PDwn",   0,          Type::Int },
            { "Pw",     0,          Type::Int }
        };

        for (auto& parameter_tuple : default_parameters)
        {
            auto key = std::get<0> (parameter_tuple);
            auto val = std::get<1> (parameter_tuple);
            auto type = std::get<2> (parameter_tuple);

            if (type == Type::Float)
            {
                set_channel_parameter(val, handle, key, channels_to_activate);
            }
            else if (type == Type::Int)
            {
                uint_fast16_t valp = (uint_fast16_t) val;
                set_channel_parameter(valp, handle, key, channels_to_activate);
            }
            else 
            {
                throw std::runtime_error("Unable to convert!");
            }
        }
    }


    void channel_manager::set_global_operating_voltage(double voltage)
    {
        if ((voltage < 0) || (voltage > 5600))
        {
            throw std::runtime_error("Voltage out of range");
        }

        set_channel_parameter(voltage, handle, "VSet", {0, 1, 2, 3});
    }

    void channel_manager::set_global_current_limit(double current)
    {
        if ((current < 0.00) || (30.00))
        {
            throw std::runtime_error("Current is out of range");
        }

        set_channel_parameter(current, handle, "ISet", {0, 1, 2, 3});
    }


    void channel_manager::set_global_voltage_limit(double voltage)
    {}

    void channel_manager::set_global_overcurrent_time_limit(double time_limit)
    {
        if ((time_limit < 0.00) || (time_limit > 1000.00))
        {
            throw std::runtime_error("Time limit out of range");
        }

        set_channel_parameter(time_limit, handle, "Trip", {0, 1, 2, 3});
    }


    void channel_manager::set_global_voltage_increase_rate(double rate)
    {
        // For safety reasons, we will limit to 1000 V/s.
        if ((rate < 0.00) || (rate > 1000.00))
        {
            throw std::runtime_error("Voltage Increase Rate out of range");
        }

        set_channel_parameter(rate, handle, "RUp", {0, 1, 2, 3});
    }

    void channel_manager::set_global_voltage_decrease_rate(double rate)
    {
        // For safety reasons, we will limit to 1000 V/s.
        if ((rate < 0.00) || (rate > 1000.00))
        {
            throw std::runtime_error("Voltage Increase Rate out of range");
        }

        set_channel_parameter(rate, handle, "RDwn", {0, 1, 2, 3});
    }


    double channel_manager::read_channel_current(int channel)
    {
    #ifndef NDEBUG
        fmt::print("\n\n----- Debugging Information -----\n");
        fmt::print("\tJust throwing away other channel readings.\n\n");
    #endif // NDEBUG

        if ( !is_an_active_channel(channel) )
        {
            std::runtime_error("Select an active channel");
        }

        auto active_channel_list = get_active_channel_numbers();

        // When we read this range in, it will either be:
        //      - 0: High Range (+/- 1 nA)
        //      - 1: Low Range (+/- 0.05 nA) 
        auto imonrange_state_vector = get_channel_parameter(
            0,
            handle,
            "ImonRange",
            active_channel_list
        );

        auto imonrange_state = imonrange_state_vector[0];

        if (imonrange_state == default_value)
        {
            std::runtime_error("Could not read IMonRange");
        }

        std::string parameter = "";
        
        // Needed so that we can get the current.
        imonrange_state ? parameter = "ImonL" : parameter = "ImonH";

        auto current_vector = get_channel_parameter(
            0.00,
            handle,
            parameter.c_str(),
            active_channel_list
        );

        double current = (double) current_vector.at(channel);
        return current;
    }

    double channel_manager::read_channel_voltage(int channel)
    {
        return -256.00;
    }

    uint_fast32_t channel_manager::read_channel_polarity(int channel)
    {
        return 256;
    }

    uint_fast32_t channel_manager::read_channel_status(int channel)
    {
        return 256;
    }


    void channel_manager::interpret_status()
    {}


    void channel_manager::enable_channel(int channel)
    {}

    void channel_manager::disable_channel(int channel)
    {}

    void channel_manager::kill_channel(int channel)
    {}

    bool channel_manager::is_an_active_channel(int channel)
    {
        auto active_channel_list = get_active_channel_numbers();

        bool is_active = std::find(
            active_channel_list.begin(),
            active_channel_list.end(),
            channel
        ) != active_channel_list.end();

        return is_active;
    }

    std::vector<uint_fast16_t> channel_manager::get_active_channel_numbers()
    {
        std::vector<uint_fast16_t> active_channel_list;
        for (auto& channel : active_channels)
        {
            if (channel.is_active)
            {
                active_channel_list.push_back(channel.number);
            }
        }

        return active_channel_list;
    }

}