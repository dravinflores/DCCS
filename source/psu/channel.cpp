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
        handle  { -1 },
        slot    { 0 },
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
        fmt::print("\nConstructor has been called on channel manager\n\n");
    #endif // NDEBUG
    }

    channel_manager::~channel_manager()
    {
    #ifndef NDEBUG
        fmt::print("\nDestructor has been called on channel manager\n\n");
    #endif // NDEBUG
    }


    void channel_manager::initialize_channels(
        int handle,
        unsigned short slot,
        std::vector<unsigned short> channels_to_activate
    )
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::initialize_channels\n");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tint handle = {}\n", handle);
        fmt::print("\tunsigned short slot = {}\n", slot);
        fmt::print("\tchannels_to_activate = [ ");

        int n = channels_to_activate.size();

        for (int i = 0; i < n - 1; ++i)
        {
            fmt::print("{}, ", channels_to_activate.at(i));
        }
        fmt::print("{} ", channels_to_activate.at(n));
        fmt::print("]\n");
    #endif // NDEBUG

        this->handle = handle;
        this->slot = slot;

        // We expect that the user will pass in the particular
        // channels to activate. For instance, if the user wants
        // CH2 and CH3 active, then the passed in vector will be
        // std::vector<unsigned short> channels { 2, 3 };

        // We want to ensure that we only activate the proper channels.
        for (auto& channel_number : channels_to_activate)
        {
            if ((channel_number < 0) || (channel_number > 3))
            {
                throw std::runtime_error("Wrong channels to initialize");
            }

            this->active_channels.at(channel_number).is_active = true;
        }

    #ifndef NDEBUG
        fmt::print("\n----- Attempting to Initialize All Channels -----\n");
    #endif // NDEBUG

        set_global_current_limit(2);
        set_global_operating_voltage(3015);

        set_global_voltage_limit(4015);
        set_global_overcurrent_time_limit(1000);

        set_global_voltage_increase_rate(1000);
        set_global_voltage_decrease_rate(1000);
        
    #ifndef NDEBUG
        fmt::print("\n----- Initialized All Channels -----\n\n");
    #endif NDEBUG
    }


    void channel_manager::set_global_current_limit(double current)
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::set_global_current_limit\n");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tdouble current = {}\n\n", current);
        fmt::print("Setting Global Current Limit");
    #endif // NDEBUG

        if ((current < 0.00) || (current > 30.00))
        {
            throw std::runtime_error("Current is out of range");
        }

        std::vector<unsigned short> channels_to_set { 0, 1, 2, 3 };

        try
        {
            set_channel_parameters(
                current,
                handle,
                "ISet",
                channels_to_set
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Global Current Limit Set Was Unsuccessful\n\n");
        #endif // NDEBUG
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "set_global_current_limit. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

    #ifndef NDEBUG
        fmt::print("Global Current Limit Set\n\n");
    #endif // NDEBUG
    }

    void channel_manager::set_global_operating_voltage(double voltage)
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::set_global_operating_voltage\n");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tdouble voltage = {}\n\n", voltage);
        fmt::print("Setting Global Operating Voltage");
    #endif // NDEBUG

        if ((voltage < 0.00) || (voltage > 4015.00))
        {
            throw std::runtime_error("Voltage is out of range");
        }

        std::vector<unsigned short> channels_to_set { 0, 1, 2, 3 };

        try 
        {    
            set_channel_parameters(
                voltage,
                handle,
                "VSet",
                channels_to_set
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Global Operating Voltage Set Was Unsuccessful\n\n");
        #endif // NDEBUG
        
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "set_global_operating_voltage. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

    #ifndef NDEBUG
        fmt::print("Global Operating Voltage Set\n\n");
    #endif // NDEBUG
    }


    void channel_manager::set_global_voltage_limit(double voltage)
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::set_global_voltage_limit\n");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tdouble voltage = {}\n\n", voltage);
        fmt::print("Setting Global Voltage Limit");
    #endif // NDEBUG

        if ((voltage < 0.00) || (voltage > 4015.00))
        {
            throw std::runtime_error("Voltage is out of range");
        }

        std::vector<unsigned short> channels_to_set { 0, 1, 2, 3 };

        try
        {    
            set_channel_parameters(
                voltage,
                handle,
                "MaxV",
                channels_to_set
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Global Voltage Limit Set Was Unsuccessful\n\n");
        #endif // NDEBUG

            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "set_global_voltage_limit. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

    #ifndef NDEBUG
        fmt::print("Global Voltage Limit Set\n\n");
    #endif // NDEBUG
    }

    void channel_manager::set_global_overcurrent_time_limit(double time_limit)
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::set_global_overcurrent_time_limit\n");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tdouble time_limit = {}\n\n", time_limit);
        fmt::print("Setting Global Overcurrent Time Limit");
    #endif // NDEBUG

        if ((time_limit < 0.00) || (time_limit > 1100.00))
        {
            throw std::runtime_error("Overcurrent time is out of range");
        }

        std::vector<unsigned short> channels_to_set { 0, 1, 2, 3 };

        try
        {    
            set_channel_parameters(
                time_limit,
                handle,
                "Trip",
                channels_to_set
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Global Over Current Limit Set Was Unsuccessful\n\n");
        #endif // NDEBUG
        
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "set_global_overcurrent_time_limit. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

    #ifndef NDEBUG
        fmt::print("Global Over Current Limit Set\n\n");
    #endif // NDEBUG
    }


    void channel_manager::set_global_voltage_increase_rate(double rate)
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::set_global_voltage_increase_rate\n");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tdouble rate = {}\n\n", rate);
        fmt::print("Setting Global Voltage Increase Rate");
    #endif // NDEBUG

        if ((rate < 0.00) || (rate > 1000.00))
        {
            throw std::runtime_error("Voltage increase rate is out of range");
        }

        std::vector<unsigned short> channels_to_set { 0, 1, 2, 3 };

        try
        {
            set_channel_parameters(
                rate,
                handle,
                "RUp",
                channels_to_set
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Global Voltage Increase Rate Set Was Unsuccessful\n\n");
        #endif // NDEBUG

            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "set_global_voltage_increase_rate. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

    #ifndef NDEBUG
        fmt::print("Global Voltage Increase Rate Set\n\n");
    #endif // NDEBUG
    }

    void channel_manager::set_global_voltage_decrease_rate(double rate)
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::set_global_voltage_decrease_rate\n");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tdouble rate = {}\n\n", rate);
        fmt::print("Setting Global Voltage Decrease Rate");
    #endif // NDEBUG

        if ((rate < 0.00) || (rate > 1000.00))
        {
            throw std::runtime_error("Voltage decrease rate is out of range");
        }

        std::vector<unsigned short> channels_to_set { 0, 1, 2, 3 };

        try
        {
            set_channel_parameters(
                rate,
                handle,
                "RDwn",
                channels_to_set
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Global Voltage Decrease Rate Set Was Unsuccessful\n\n");
        #endif // NDEBUG
        
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "set_global_voltage_decrease_rate. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

    #ifndef NDEBUG
        fmt::print("Global Voltage Decrease Rate Set\n\n");
    #endif // NDEBUG
    }


    double channel_manager::read_channel_current(unsigned short channel)
    {
    #ifndef NDEBUG
        fmt::print("\nchannel_manager::read_channel_current");
        fmt::print("Arguments passed in: \n");
        fmt::print("\tunsigned short channel = {}\n", channel);
    #endif // NDEBUG

        fmt::print("Currently ignoring active channels.\n");
        /*
        if ( !is_an_active_channel(channel) )
        {
            std::runtime_error("Select an active channel");
        }
        */

        // auto active_channel_list = get_active_channel_numbers();
        std::vector<unsigned short> active_channel_list = { channel };
        unsigned long type_hint = default_value;

        // When we read this range in, it will either be:
        //      - 0: High Range (+/- 1 nA)
        //      - 1: Low Range (+/- 0.05 nA) 

        std::vector<unsigned long> imonrange_state_vector;
        try
        {    
            imonrange_state_vector = get_channel_parameters(
                type_hint,
                handle,
                "Imonrange",
                active_channel_list
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Unable to read Imonrange\n\n");
        #endif // NDEBUG
        
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "read_channel_current. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

        auto imonrange_state = imonrange_state_vector[0];

        if (imonrange_state == default_value)
        {
            std::runtime_error("Could not read ImonRange");
        }

        std::string parameter = "";
        
        // Needed so that we can get the current.
        imonrange_state ? parameter = "IMonL" : parameter = "IMonH";

        std::vector<double> current_vector;
        double secondary_type_hint = 0.00;

        try
        {
            current_vector = get_channel_parameters(
                secondary_type_hint,
                handle,
                parameter.c_str(),
                active_channel_list
            );
        }
        catch (std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Unable to read current zoom\n\n");
        #endif // NDEBUG
        
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "read_channel_current. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }

        double current = (double) current_vector[0];
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

        std::vector<unsigned short> channels_to_set { 0 };
        unsigned long power_on = 1;

        try
        {
            set_channel_parameters(power_on, handle, "Pw", channels_to_set);
        }
        catch(std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Unable to enable channel\n\n");
        #endif // NDEBUG
        
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "enable_channel. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }
    }

    void channel_manager::disable_channel(unsigned short channel)
    {
    #ifndef NDEBUG
        fmt::print("Enabling channel\n");
    #endif // NDEBUG

        std::vector<unsigned short> channels_to_set { 0 };
        unsigned long power_off = 0;
        
        try
        {
            set_channel_parameters(power_off, handle, "Pw", channels_to_set);
        }
        catch(std::runtime_error& error)
        {
        #ifndef NDEBUG
            fmt::print("Unable to disable channel\n\n");
        #endif // NDEBUG
        
            std::string error_str = fmt::format(
                "Caught set_channel_parameters error in "
                "disable_channel. Propagating out. "
                "{}",
                error.what()
            );
            throw std::runtime_error(error_str.c_str());
        }
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