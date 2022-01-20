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
    channel::channel()
    {}

    channel::channel(interchannel info, int number) 
    {
        fmt::print("Constructor called!\n");
        this->info = info;
        this->channel_number = number;

        // These values are the default. We should read this from a
        // configuration file, such as a TOML file.
        // TODO: Read-in using a TOML file.
        std::map<std::string, double> default_parameter_values {
            { "VSet",   3015.00 },
            { "ISet",   2.00 },
            { "MaxV",   4015.00 },
            { "RUp",    500.00 },
            { "RDwn",   500.00 },
            { "Trip",   1000.00 },
            { "PDwn",   0 },
            { "Pw",     0 }
        };

        // We're going to iteratively set these default parameters.
        for (auto& [key, val] : default_parameter_values)
        {
            CAENHVRESULT result = -55;

            // For any of the booleans.
            if ( (int) val == 0 ) 
            {
                int casted_val = (int) val;
                result = CAENHV_SetChParam(
                    this->info.handle,
                    this->info.slot,
                    key.c_str(),
                    this->channel_number,
                    this->info.channel_list.data(),
                    &casted_val
                );
            }
            else 
            {    
                result = CAENHV_SetChParam(
                    this->info.handle,
                    this->info.slot,
                    key.c_str(),
                    this->channel_number,
                    this->info.channel_list.data(),
                    &val
                );
            }

            if (result != CAENHV_OK)
            {
                std::string error = fmt::format(
                    "CAENHV_SetChParam() error: {}",
                    CAENHV_GetError(this->info.handle) 
                );

                throw std::runtime_error(error.c_str());
            }
        }

        bool m_is_using_zero_current_adjust = false;
        double intrinsic_current = 0.00;

    #ifndef NDEBUG
        fmt::print("Have not implemented a way to obtain parasitic current\n");
    #endif // NDEBUG
    }

    /*
    channel::channel(channel&& move) noexcept
    {}
    */

    channel::channel(const channel& copy):
        info                            { copy.info },
        name                            { copy.name },
        channel_number                  { copy.channel_number },
        m_is_using_zero_current_adjust  { m_is_using_zero_current_adjust },
        intrinsic_current               { intrinsic_current }
    {}

    /*
    channel& channel::operator=(channel&& move) noexcept
    {
        if (&move != this)
        {

        }
        return *this;
    }
    */

    channel& channel::operator=(const channel& copy)
    {
        // Checking for self assignment.
        if (this != &copy)
        {
            info                            = copy.info;
            name                            = copy.name;
            channel_number                  = copy.channel_number;
            m_is_using_zero_current_adjust  = m_is_using_zero_current_adjust;
            intrinsic_current               = intrinsic_current;
        }

        return *this;
    }

    
    channel::~channel()
    {
    #ifndef NDEBUG
        fmt::print("Destructor called.\n");
    #endif // NDEBUG
    }
    

    void channel::set_voltage(double voltage)
    {
        if ((voltage < 0) || (voltage > 5600))
        {
            throw std::runtime_error("Voltage out of range");
        }

        CAENHVRESULT result = CAENHV_SetChParam(
            this->info.handle,
            this->info.slot,
            "VSet",
            this->channel_number,
            this->info.channel_list.data(),
            &voltage
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }
    }
    
    double channel::read_voltage()
    {
        double voltage = -255.00;

        CAENHVRESULT result = CAENHV_GetChParam(
            this->info.handle,
            this->info.slot,
            "VMon",
            this->channel_number,
            this->info.channel_list.data(),
            &voltage
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }

        // There is a parameter called MaxV.
        // TODO: Consider using MaxV value.
        if (voltage < 0.00)
        {
            throw std::runtime_error("Unable to read voltage");
        }

        return voltage;
    }

    void channel::set_current(double current)
    {
        if ((current < 0.00) || (30.00))
        {
            throw std::runtime_error("Current is out of range");
        }

        CAENHVRESULT result = CAENHV_GetChParam(
            this->info.handle,
            this->info.slot,
            "ISet",
            this->channel_number,
            this->info.channel_list.data(),
            &current
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }
    }
    
    double channel::get_current()
    {
        // When we read this range in, it will either be:
        //      0: High Range (+/- 1 nA)
        //      1: Low Range (+/- 0.05 nA) 
        unsigned int imonrange_state;

        // We need to check the zoom level for the current.
        CAENHVRESULT result = CAENHV_GetChParam(
            this->info.handle,
            this->info.slot,
            "ImonRange",
            this->channel_number,
            this->info.channel_list.data(),
            &imonrange_state
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }

        std::string parameter = "";
        if (imonrange_state)
        {
            parameter = "IMonL";
        }
        else
        {
            parameter = "IMonH";
        }

        double current = -255.00;
        CAENHVRESULT current_reading_result = CAENHV_GetChParam(
            this->info.handle,
            this->info.slot,
            parameter.c_str(),
            this->channel_number,
            this->info.channel_list.data(),
            &current
        );

        if (current_reading_result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }

        // Here we would adjust for the parasitic current.
        if (this->m_is_using_zero_current_adjust)
        {
            current -= this->intrinsic_current;
        }

        if (current < 0.00)
        {
            throw std::runtime_error("Current out of range");
        }

        return current;
    }

    polarity channel::get_polarity()
    {
        unsigned int retrieved_polarity;

        CAENHVRESULT result = CAENHV_SetChParam(
            info.handle,
            info.slot,
            "Polarity",
            channel_number,
            info.channel_list.data(),
            &retrieved_polarity
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }

        polarity return_polarity;

        if (retrieved_polarity)
        {
            return_polarity = polarity::reverse;
        }
        else
        {
            return_polarity = polarity::normal;
        }

        return return_polarity;
    }


    void channel::set_voltage_limit(double voltage)
    {}
    
    void channel::set_overcurrent_time_limit(double time_limit)
    {
        if ((time_limit < 0.00) || (time_limit > 1000.00))
        {
            throw std::runtime_error("Time limit out of range");
        }

        CAENHVRESULT result = CAENHV_SetChParam(
            this->info.handle,
            this->info.slot,
            "Trip",
            this->channel_number,
            this->info.channel_list.data(),
            &time_limit
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }
    }
    

    void channel::set_voltage_increase_rate(double rate)
    {
        // For safety reasons, we will limit to 1000 V/s.
        if ((rate < 0.00) || (rate > 1000.00))
        {
            throw std::runtime_error("Voltage Increase Rate out of range");
        }

        CAENHVRESULT result = CAENHV_SetChParam(
            this->info.handle,
            this->info.slot,
            "RUp",
            this->channel_number,
            this->info.channel_list.data(),
            &rate
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }
    }
    
    void channel::set_voltage_decrease_rate(double rate)
    {
        // For safety reasons, we will limit to 1000 V/s.
        if ((rate < 0.00) || (rate > 1000.00))
        {
            throw std::runtime_error("Voltage Decrease Rate out of range");
        }

        CAENHVRESULT result = CAENHV_SetChParam(
            this->info.handle,
            this->info.slot,
            "RDwn",
            this->channel_number,
            this->info.channel_list.data(),
            &rate
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }
    }

    
    void channel::adjust_for_zero_current()
    {
        fmt::print("Yeah...about that. Can we raincheck this?\n");
    }
    
    bool channel::is_using_zero_current_adjust()
    {
        return this->m_is_using_zero_current_adjust;
    }


    void channel::read_status()
    {
        // Here are the bits defined in the power supply manual.
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

    void channel::power_on()
    {
        unsigned int power = 1;
        CAENHVRESULT result = CAENHV_SetChParam(
            this->info.handle,
            this->info.slot,
            "Pw",
            this->channel_number,
            this->info.channel_list.data(),
            &power
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }
    }

    void channel::power_off()
    {
        unsigned int power = 0;
        CAENHVRESULT result = CAENHV_SetChParam(
            this->info.handle,
            this->info.slot,
            "Pw",
            this->channel_number,
            this->info.channel_list.data(),
            &power
        );

        if (result != CAENHV_OK)
        {
            std::string error = fmt::format(
                "CAENHV_SetChParam() error: {}",
                CAENHV_GetError(this->info.handle) 
            );

            throw std::runtime_error(error.c_str());
        }
    }

    void channel::kill()
    {
    #ifndef NDEBUG
        fmt::print("\n\nKILLING THE CHANNEL\n\n");
    #endif // NDEBUG
    }
}