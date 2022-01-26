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
    static constexpr int default_value { 512 };

    channel_manager::channel_manager():
        handle { -1 },
        slot { 0 },
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
        int handle,
        unsigned short slot,
        std::vector<unsigned short> channels_to_activate
    )
    {
        this->handle = handle;
        this->slot = slot;

        // We expect that the user will pass in the particular
        // channels to activate. For instance, if the user wants
        // CH2 and CH3 active, then the passed in vector will be
        // std::vector<unsigned short> channels { 2, 3 };

        // We want to ensure that we only activate the proper channels.
        for (auto& channel_number : channels_to_activate)
        {
            this->active_channels.at(channel_number).is_active = true;
        }

    #ifndef NDEBUG
        fmt::print("\n----- Attempting to Initialize All Channels -----\n");
    #endif // NDEBUG

        // set_global_current_limit(2);
        // set_global_operating_voltage(3015);

        // set_global_voltage_limit(4015);
        // set_global_overcurrent_time_limit(1000);

        // set_global_voltage_increase_rate(1000);
        // set_global_voltage_decrease_rate(1000);
        
    #ifndef NDEBUG
        fmt::print("\n----- Initialized All Channels -----\n\n");
    #endif NDEBUG
    }


    void channel_manager::set_global_current_limit(double current)
    {
    #ifndef NDEBUG
        fmt::print("\tSetting Global Current Limit");
    #endif // NDEBUG

        if ((current < 0.00) || (current > 30.00))
        {
            throw std::runtime_error("Current is out of range");
        }

        std::vector<unsigned short> passed_in_list { 0, 1, 2, 3 };
        set_channel_parameters(
            current,
            handle,
            slot,
            "ISet",
            passed_in_list
        );

    #ifndef NDEBUG
        fmt::print("{>30}\n", "Done");
    #endif // NDEBUG
    }

    void channel_manager::set_global_operating_voltage(double voltage)
    {}


    void channel_manager::set_global_voltage_limit(double voltage)
    {}

    void channel_manager::set_global_overcurrent_time_limit(double time_limit)
    {}


    void channel_manager::set_global_voltage_increase_rate(double rate)
    {}

    void channel_manager::set_global_voltage_decrease_rate(double rate)
    {}


    double channel_manager::read_channel_current(unsigned short channel)
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
        unsigned long type_hint = default_value;

        // When we read this range in, it will either be:
        //      - 0: High Range (+/- 1 nA)
        //      - 1: Low Range (+/- 0.05 nA) 

        std::vector<unsigned short> passed_in_list { 0, 1, 2, 3 };
        auto imonrange_state_vector = get_channel_parameters(
            type_hint,
            handle,
            slot,
            "ImonRange",
            passed_in_list
        );

        auto imonrange_state = imonrange_state_vector[0];

        if (imonrange_state == default_value)
        {
            std::runtime_error("Could not read IMonRange");
        }

        std::string parameter = "";
        
        // Needed so that we can get the current.
        imonrange_state ? parameter = "ImonL" : parameter = "ImonH";

        auto current_vector = get_channel_parameters(
            0.00,
            handle,
            slot,
            parameter.c_str(),
            passed_in_list
        );

        double current = (double) current_vector.at(channel);
        return current;
    }

    double channel_manager::read_channel_voltage(unsigned short channel)
    {
        return default_value;
    }

    uint_fast32_t channel_manager::read_channel_status(unsigned short channel)
    {
        return (uint_fast32_t) default_value;
    }

    uint_fast32_t channel_manager::read_channel_polarity(unsigned short channel)
    {
        return (uint_fast32_t) default_value;
    }


    void channel_manager::interpret_status()
    {}


    void channel_manager::enable_channel(unsigned short channel)
    {
    #ifndef NDEBUG
        fmt::print("Enabling channel\n");
    #endif // NDEBUG

        std::vector<unsigned short> passed_in_list { 0 };
        unsigned long power_on = 1;
        set_channel_parameters(power_on, handle, slot, "Pw", passed_in_list);
    }

    void channel_manager::disable_channel(unsigned short channel)
    {
    #ifndef NDEBUG
        fmt::print("Enabling channel\n");
    #endif // NDEBUG

        std::vector<unsigned short> passed_in_list { 0 };
        unsigned long power_off = 0;
        set_channel_parameters(power_off, handle, slot, "Pw", passed_in_list);
    }

    void channel_manager::kill_channel(unsigned short channel)
    {}


    bool channel_manager::is_an_active_channel(unsigned short channel)
    {
        auto active_channel_list = get_active_channel_numbers();

        bool is_active = std::find(
            active_channel_list.begin(),
            active_channel_list.end(),
            channel
        ) != active_channel_list.end();

        return is_active;
    }

    std::vector<unsigned short> channel_manager::get_active_channel_numbers()
    {
        std::vector<unsigned short> active_channel_list;
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