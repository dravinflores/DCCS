/***********************************************************************************************************************
 *  File:           psu/hv_interface.cpp
 *  Author:         Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   30 May 2022
 * 
 *  Purpose:        This source file defines the functions for the hv_interface. 
 * 
 *  Known Issues:
 * 
 *  Workarounds:
 * 
 *  Updates:
 **********************************************************************************************************************/

#include <psu/hv_interface.hpp>

static constexpr int default_value { 512 };

hv_interface::hv_interface():
    handle              { default_value },
    connection_status   { false },
    logger              { spdlog::get("hvlib_logger") }
{}

hv_interface::~hv_interface()
{}

#ifdef VIRTUALIZE_CONNECTION

void hv_interface::connect(const msu_smdt::com_port& port)
{
    logger->debug("Function connect() called.");
}

void hv_interface::disconnect()
{
    logger->debug("Function disconnect() called.");
}

bool hv_interface::is_connected_to_power_supply()
{
    logger->debug("Function get_connection_status() called.");
    return false;
}

void hv_interface::set_parameter_float(float val, std::string parameter, unsigned short channel)
{
    logger->debug("Function set_parameter_float() called.");
}

void hv_interface::set_parameter_long(unsigned long val, std::string parameter, unsigned short channel)
{
    logger->debug("Function set_parameter_long() called.");
}

float hv_interface::get_parameter_float(std::string parameter, unsigned short channel)
{
    logger->debug("Function get_parameter_float() called.");
    return (float) default_value;
}

unsigned long hv_interface::get_parameter_long(std::string parameter, unsigned short channel)
{
    logger->debug("Function get_parameter_long() called.");
    return (unsigned long) default_value;
}

#else

#include <fmt/core.h>

#include <CAENHVWrapper.h>

void hv_interface::get_crate_map()
{
    unsigned short number_of_slots = (unsigned short) default_value;
    unsigned short* number_of_channels_list = nullptr;
    char* model_list = nullptr;
    char* description_list = nullptr;
    unsigned short* serial_number_list = nullptr;
    unsigned char* firmware_release_minimum_list = nullptr;
    unsigned char* firmware_release_max_list = nullptr;

    // The documentation for the HV Wrapper Library specifically mentions
    // calling this function DIRECTLY AFTER initialization.
    CAENHVRESULT crate_map_result = CAENHV_GetCrateMap(
        handle,
        &number_of_slots,
        &number_of_channels_list,
        &model_list,
        &description_list,
        &serial_number_list,
        &firmware_release_minimum_list,
        &firmware_release_max_list
    );

    // We need to check for any invalid allocations.
    if ((number_of_slots == (unsigned short) default_value)
        || (number_of_channels_list == nullptr) 
        || (model_list == nullptr) 
        || (description_list == nullptr) 
        || (serial_number_list == nullptr)
        || (firmware_release_minimum_list == nullptr)
        || (firmware_release_max_list == nullptr)
        || crate_map_result != CAENHV_OK)
    {
        // If we have an invalid allocation, we need to iterate through and 
        // free all the rest of the memory. Then, we will alert the caller. 
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
            const std::string hvlib_error = CAENHV_GetError(handle);
            message = fmt::format("CAENHV_GetCrateMap Error: {}", hvlib_error);
        }

        disconnect();
        logger->error("{}", message);
    }

    // Now that we're at this point, we should have properly allocated
    // all the arguments passed in. Normally, we have to run through
    // these lists iteratively. However, we can leverage the fact that
    // we know what board we are dealing with.
    logger->debug("\tBoard #{}", 0);
    logger->debug("\tModel: {}", model_list);
    logger->debug("\tDescription: {}", description_list);
    logger->debug("\tChannels available: {}", number_of_channels_list[0]);
    logger->debug("\tSerial: {}", serial_number_list[0]);
    logger->debug("\tFirmware: {}.{}", firmware_release_max_list[0], firmware_release_minimum_list[0]);

    // Should we do something with these?
    int slot = (int) number_of_slots;
    int number_of_channels = (int) number_of_channels_list[0];
    int serial_number = (int) serial_number_list[0];
    auto firmware_version = fmt::format("{}.{}", firmware_release_max_list[0], firmware_release_minimum_list[0]);

    // No clue how this works internally. 
    CAENHV_Free(number_of_channels_list);
    CAENHV_Free(model_list);
    CAENHV_Free(description_list);
    CAENHV_Free(serial_number_list);
    CAENHV_Free(firmware_release_minimum_list);
    CAENHV_Free(firmware_release_max_list);
}

void hv_interface::set_parameter_primitive(value new_val, type type, std::string parameter, unsigned short channel)
{
    unsigned short slot = 0;
    const char* parameter_name_casted = (const char*) parameter.data();
    unsigned short number_of_channels = 1;

    void* address_of_parameter;
    if (type == type::FLOAT)
    {
        float f = new_val.f;
        address_of_parameter = (void*) &f;
    }
    else
    {
        unsigned long l = new_val.l;
        address_of_parameter = (void*) l;
    }

    // The HVLIB expects a list of channels. We'll go ahead and just use the
    // pointer decay of an array to pass this in.
    unsigned short channel_list[1] = { channel };

    CAENHVRESULT result = CAENHV_SetChParam(
        handle,
        slot,
        parameter_name_casted,
        number_of_channels,
        channel_list,
        address_of_parameter
    );

    if (result != CAENHV_OK)
    {
        std::string hvlib_error = CAENHV_GetError(handle);
        logger->error("Unable to set channel parameter: {}", hvlib_error);
    }
    else
    {
        logger->debug("Parameter {} was set", parameter);
    }
}

hv_interface::value hv_interface::get_parameter_primitive(type type, std::string parameter, unsigned short channel)
{
    unsigned short slot = 0;
    const char* parameter_name_casted = (const char*) parameter.data();
    unsigned short number_of_channels = 1;
    unsigned short channel_list[1] = { channel };

    unsigned long storage_l = (unsigned long) default_value;
    float storage_f = (float) default_value;

    int result = default_value;

    if (type == hv_interface::type::FLOAT)
    {
        result = CAENHV_GetChParam(
            handle,
            slot,
            parameter_name_casted,
            number_of_channels,
            channel_list,
            (void*) &storage_f
        );
    }
    else
    {
        result = CAENHV_GetChParam(
            handle,
            slot,
            parameter_name_casted,
            number_of_channels,
            channel_list,
            (void*) &storage_l
        );
    }

    if (result != CAENHV_OK)
    {
        std::string hvlib_error = CAENHV_GetError(handle);
        logger->error("Unable to get channel parameter: {}", hvlib_error);
        logger->debug("Returning default value: {}", default_value);
    }
    else
    {
        logger->debug("Parameter read as {}", parameter);
    }

    hv_interface::value return_val;

    if (type == hv_interface::type::FLOAT)
    {
        return_val.f = storage_f;
    }
    else
    {
        return_val.l = storage_l;
    }

    return return_val;
}

void hv_interface::connect(const msu_smdt::com_port& port)
{
    // We need to check that the proper com_port struct was passed in.
    bool is_empty = port.port.empty()   \
        || port.baud_rate.empty()       \
        || port.data_bit.empty()        \
        || port.stop_bit.empty()        \
        || port.parity.empty()          \
        || port.lbusaddress.empty();

    std::string port_str = fmt::format(
        "{}_{}_{}_{}_{}_{}", 
        port.port,
        port.baud_rate, 
        port.data_bit,
        port.stop_bit,
        port.parity,
        port.lbusaddress
    );

    // Here are some variables specific to the power supply initialization.
    int handle = -1;
    CAENHV_SYSTEM_TYPE_t system = N1470;
    int link_type = LINKTYPE_USB_VCP;
    std::string username = "";
    std::string password = "";

    // Now we need to make the connection to the power supply.
    CAENHVRESULT result = CAENHV_InitSystem(
        system, 
        link_type, 
        (void*) port_str.c_str(), 
        username.c_str(), 
        password.c_str(),
        &handle
    );

    if (result != CAENHV_OK)
    {
        std::string hvlib_error = CAENHV_GetError(handle);
        logger->error("Unable to initialize system. Error: {}", hvlib_error);
    }
    else
    {
        logger->debug("System initialized. Handle: {}", handle);
        connection_status = true;
        get_crate_map();
    }
}

void hv_interface::disconnect()
{
    CAENHVRESULT result = CAENHV_DeinitSystem(handle);

    if (result != CAENHV_OK)
    {
        std::string hvlib_error = CAENHV_GetError(handle);
        logger->error("Unable to deinitialize system. Error: {}", hvlib_error);
    }
    else
    {
        logger->debug("Parameter de-initialized");
        connection_status = false;
    }
}

bool hv_interface::is_connected_to_power_supply()
{
    return connection_status;
}

void hv_interface::set_parameter_float(float val, std::string parameter, unsigned short channel)
{
    value new_val;
    new_val.f = val;
    set_parameter_primitive(new_val, hv_interface::type::FLOAT, parameter, channel);
}

void hv_interface::set_parameter_long(unsigned long val, std::string parameter, unsigned short channel)
{
    value new_val;
    new_val.l = val;
    set_parameter_primitive(new_val, hv_interface::type::UNSIGNED, parameter, channel);
}

float hv_interface::get_parameter_float(std::string parameter, unsigned short channel)
{
    value return_val = get_parameter_primitive(hv_interface::type::FLOAT, parameter, channel);
    return return_val.f;
}

unsigned long hv_interface::get_parameter_long(std::string parameter, unsigned short channel)
{
    value return_val = get_parameter_primitive(hv_interface::type::UNSIGNED, parameter, channel);
    return return_val.l;
}

#endif