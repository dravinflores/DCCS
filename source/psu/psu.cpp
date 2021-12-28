/*******************************************************************************
 *  File:           PSU/psu.cpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   17 December, 2021
 * 
 *  Purpose:        This file defines all the functions prototyped in
 *                  the psu.hpp header file.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 ******************************************************************************/

#include <psu.hpp>

msu_smdt::psu::psu(const msu_smdt::com_port& com_port_info)
{
    // We need to check that the proper com_port struct was passed in.
    bool is_empty = com_port_info.port.empty()                  \
        || com_port_info.baud_rate.empty()                      \
        || com_port_info.data_bit.empty()                       \
        || com_port_info.stop_bit.empty()                       \
        || com_port_info.parity.empty()                         \
        || com_port_info.lbusaddress.empty();

    // We should probably put an assert somewhere here.
    // assert(!is_empty)

#ifndef NDEBUG
    fmt::print(
        "Here in the file {} on line {}, is_empty = {}.\n", 
        __FILE__, 
        __LINE__, 
        is_empty
    );
#endif // NDEBUG

    // The CAEN HV Wrapper library expects the following format:
    // port_baudrate_data_stop_parity_lbusaddress.
    this->com_port_str = fmt::format(
        "{}_{}_{}_{}_{}_{}", 
        com_port_info.port,
        com_port_info.baud_rate, 
        com_port_info.data_bit,
        com_port_info.stop_bit,
        com_port_info.parity,
        com_port_info.lbusaddress
    );

#ifndef NDEBUG
    fmt::print("We want to see if we can link to the CAENHVWrapper library.\n");
#endif // NDEBUG

    this->handle = -1;
    CAENHV_SYSTEM_TYPE_t system = N1470;
    int link_type = LINKTYPE_USB_VCP;
    std::string username = "";
    std::string password = "";

    CAENHVRESULT result = CAENHV_InitSystem(
        system, 
        link_type, 
        (void*) com_port_str.c_str(), 
        username.c_str(), 
        password.c_str(),
        &handle
    );

#ifndef NDEBUG
    fmt::print("Attempting connection. Result is {}.\n", result);
#endif // NDEBUG

    // Made to be more explicit.
    if (result != CAENHV_OK)
    {    
        std::string error = CAENHV_GetError(this->handle);

        // We'll probably want to log here rather than printing to STDOUT.

    #ifndef NDEBUG
        fmt::print("\tAttempting to read error: {}\n", error);
    #endif // NDEBUG

        // We need the creator of this object to catch this error.
        throw std::runtime_error(error.c_str());
    }

    // If we've reached this point, then we've successfully
    // connected to the PSU.

    // TODO: Need a better system. This is kinda clunky.
    unsigned short number_of_slots = -1;
    unsigned short* number_of_channels_list;
    char* model_list;
    char* description_list;
    unsigned short* serial_number_list;
    unsigned char* firmware_release_minimum_list;
    unsigned char* firmware_release_max_list;

    CAENHVRESULT crate_map_result = CAENHV_GetCrateMap(
        this->handle,
        &number_of_slots,
        &number_of_channels_list,
        &model_list,
        &description_list,
        &serial_number_list,
        &firmware_release_minimum_list,
        &firmware_release_max_list
    );

    // We need to check for any invalid allocations.

    if ((number_of_slots == -1)
        || (number_of_channels_list == nullptr) 
        || (model_list == nullptr) 
        || (description_list == nullptr) 
        || (serial_number_list == nullptr)
        || (firmware_release_minimum_list == nullptr)
        || (firmware_release_max_list == nullptr)
        || crate_map_result != CAENHV_OK)
    {
    #ifndef NDEBUG
        fmt::print("\n\n\t\t Invalid Allocation!\n\n");
    #endif // NDEBUG

        // If we have an invalid allocation, we need to iterate through
        // and free all the rest of the memory. Then, we will alert the caller.
        // Note: I have no idea how these work under the hood.
        if (number_of_channels_list != nullptr) 
        {
            CAENHV_Free(number_of_channels_list);
        }
        if (model_list != nullptr) 
        {
            CAENHV_Free(model_list);
        }
        if (description_list != nullptr) 
        {
            CAENHV_Free(description_list);
        }
        if (serial_number_list != nullptr)
        {
            CAENHV_Free(serial_number_list);
        }
        if (firmware_release_minimum_list != nullptr)
        {
            CAENHV_Free(firmware_release_minimum_list);
        }
        if (firmware_release_max_list != nullptr)
        {
            CAENHV_Free(firmware_release_max_list);
        }

        std::string message;

        if (crate_map_result == CAENHV_OK)
        {
            message = "Invalid allocation for arguments to CAENHV_GetCrateMap";
        }
        else
        {
            const std::string error = CAENHV_GetError(this->handle);
            message = fmt::format("CAENHV_GetCrateMap Error: {}", error);
        }

        // Now we need to deinitialize the PSU before we throw.
        // We should handle the case when we cannot deinitialize the PSU.
        CAENHVRESULT result = CAENHV_DeinitSystem(this->handle);

        throw std::runtime_error(message.c_str());
    }

    // Now that we're at this point, we should have properly allocated
    // all the arguments passed in. Normally, we have to run through
    // these lists iteratively. However, we can leverage the fact that
    // we know what board we are dealing with.
#ifndef NDEBUG
    fmt::print("\tBoard #{}\n", 0);
    fmt::print("\tModel: {}\n", model_list);
    fmt::print("\tDescription: {}\n", description_list);
    fmt::print("\tChannels available: {}\n", number_of_channels_list[0]);
    fmt::print("\tSerial: {}\n", serial_number_list[0]);
    fmt::print(
        "\tFirmware: {}.{}", 
        firmware_release_max_list[0], 
        firmware_release_minimum_list[0]
    );
#endif // NDEBUG

    this->slot = (int) number_of_slots;
    this->number_of_channels = (int) number_of_channels_list[0];
    this->serial_number = (int) serial_number_list[0];
    this->firmware_version = fmt::format(
        "{}.{}", firmware_release_max_list[0], firmware_release_minimum_list[0]
    );

    // No clue how this works internally. 
    CAENHV_Free(number_of_channels_list);
    CAENHV_Free(model_list);
    CAENHV_Free(description_list);
    CAENHV_Free(serial_number_list);
    CAENHV_Free(firmware_release_minimum_list);
    CAENHV_Free(firmware_release_max_list);
}


msu_smdt::psu::~psu()
{
    print_internal_com_port_string();
    CAENHVRESULT result = CAENHV_DeinitSystem(this->handle);

#ifndef NDEBUG
    // So that we can have a prettier STDOUT message.
    fmt::print("\n");
#endif // NDEBUG
}


void msu_smdt::psu::print_internal_com_port_string()
{
    fmt::print("{}\n", this->com_port_str);
}


int msu_smdt::psu::get_number_of_channels()
{
    return number_of_channels;
}


void msu_smdt::psu::set_number_of_channels(int number_of_channels)
{
    // Need to actually wrap this into a Qt message or something.
    if (number_of_channels > 4)
    {
        fmt::print("This PSU only supports 4 channels: CH0, CH1, CH2, CH3.\n");
        fmt::print("Please try again.\n");
    }
    else
    {
        this->number_of_channels = number_of_channels;

        for (int i = 0; i < number_of_channels; ++i)
        {
            // assert sizeof(this->channel_list) == number_of_channels
            this->channel_list.at(i) = i;
        }
    }
}


msu_smdt::polarity msu_smdt::psu::get_channel_polarity(int channel)
{
    // Need to check that the channel being passed in is valid.
    if (channel > number_of_channels)
    {
        throw std::runtime_error("Invalid channel number");
    }

    /*
    auto result = auto result = CAENHV_GetChParam(
        this->handle,
        this->slot,
        "POL",
        channel,

    )
    */
}


void msu_smdt::psu::power_on_channel(int channel)
{
    if (channel > number_of_channels)
    {
        throw std::runtime_error("Invalid channel number");
    }

    // Now we need to check the status of the channel (i.e. if the channel
    // is in over-voltage-protection mode).
}


void msu_smdt::psu::power_off_channel(int channel)
{}


void msu_smdt::psu::kill_all()
{
    fmt::print("\n");

    for (int i = 0; i < number_of_channels; ++i)
    {
        fmt::print("Attempting to turn off channel {}\n", i);
        power_off_channel(i);
    }

    fmt::print("\n");
}


void msu_smdt::psu::enable_all()
{
#ifndef NDEBUG
    fmt::print("\n");
#endif // NDEBUG

    for (int i = 0; i < number_of_channels; ++i)
    {
    #ifndef NDEBUG
        fmt::print("Attempting to turn on channel {}\n", i);
    #endif // NDEBUG
        power_on_channel(i);
    }

#ifndef NDEBUG
    fmt::print("\n");
#endif // NDEBUG
}


void msu_smdt::psu::disable_all()
{
#ifndef NDEBUG
    fmt::print("\n");
#endif // NDEBUG

    for (int i = 0; i < number_of_channels; ++i)
    {
    #ifndef NDEBUG
        fmt::print("Attempting to turn off channel {}\n", i);
    #endif // NDEBUG
        power_off_channel(i);
    }

#ifndef NDEBUG
    fmt::print("\n");
#endif // NDEBUG
}


double msu_smdt::psu::read_current(int from_channel)
{
    auto channel = from_channel; 
    return 0.00;
}


double msu_smdt::psu::read_voltage(int from_channel)
{
    auto channel = from_channel;
    return 0.00;
}


void msu_smdt::psu::adjust_for_intrinsic_current()
{
    // Ask the DCCHCONOBJ to disconnect all channels physically.
    
    enable_all();

    while ((int) read_voltage(0) != 3015)
    {
        // Sleep.
        fmt::print("Sleep\n");
    }

    std::vector<double> channel_intrinsic_currents;

    for (int i = 0; i < number_of_channels; ++i)
    {
        channel_intrinsic_currents.push_back(read_current(i));
    }

    this->channel_intrinsic_currents = channel_intrinsic_currents;

    disable_all();
}


void msu_smdt::psu::start_test(int)
{}