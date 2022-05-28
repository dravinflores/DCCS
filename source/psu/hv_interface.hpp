/***********************************************************************************************************************
 *  File:           psu/hv_interface.cpp
 *  Author:         Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   27 May 2022
 * 
 *  Purpose:        This source file defines the functions for the hv_interface. 
 *                  In order to isolate the actual connection to the HVLIB, this
 *                  file uses many internal functions not defined in the
 *                  header file. 
 * 
 *  Known Issues:
 *                  Every function calls spdlog::get(), which causes a serious
 *                  bottleneck for the program as a whole. It would be better
 *                  to store that shared pointer somewhere. 
 * 
 *                  Also, it feels kinda weird to have function definitions in
 *                  the header file. Given that only a single class should
 *                  ever include this header, it doesn't seem too far fetched
 *                  to keep the design this way.
 *                      
 * 
 *  Workarounds:
 * 
 *  Updates:
 **********************************************************************************************************************/

#pragma once

#include <fmt/core.h>
#include <spdlog/spdlog.h>

// #include <psu/hv_interface.hpp>

#ifdef VIRTUALIZE_CONNECTION
int initialize_system(const msu_smdt::com_port& com_port_info)
{
    auto logger = spdlog::get("hvlib_logger");
    logger->debug("Function initialize_system() called");
    return -1;
}

void deinitialize_system(int handle)
{
    auto logger = spdlog::get("hvlib_logger");
    logger->debug("Function deinitialize_system() called");
}


std::tuple<int, int, int, std::string> get_crate_map(int handle)
{
    auto logger = spdlog::get("hvlib_logger");
    logger->debug("Function get_crate_map() called");
    return { -1, -1, -1, "VIRT" };
}


template <typename T>
void set_channel_parameter(T val, int handle, std::string parameter, unsigned short channel_number)
{
    auto logger = spdlog::get("hvlib_logger");
    logger->debug("Function set_channel_parameter() called");
}


template <typename T>
T get_channel_parameter(int handle, std::string parameter, unsigned short channel_number)
{
    auto logger = spdlog::get("hvlib_logger");
    logger->debug("Function get_channel_parameter() called");
    
    T dummy_thing = (T) 100;
    return dummy_thing;
}

#else // VIRTUALIZE_CONNECTION 

extern "C" {
#include <CAENHVWrapper.h>
}

int initialize_system(const msu_smdt::com_port& com_port_info)
{
    // We need to check that the proper com_port struct was passed in.
        bool is_empty = com_port_info.port.empty()              \
            || com_port_info.baud_rate.empty()                  \
            || com_port_info.data_bit.empty()                   \
            || com_port_info.stop_bit.empty()                   \
            || com_port_info.parity.empty()                     \
            || com_port_info.lbusaddress.empty();

        std::string com_port_str = fmt::format(
            "{}_{}_{}_{}_{}_{}", 
            com_port_info.port,
            com_port_info.baud_rate, 
            com_port_info.data_bit,
            com_port_info.stop_bit,
            com_port_info.parity,
            com_port_info.lbusaddress
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
            (void*) com_port_str.c_str(), 
            username.c_str(), 
            password.c_str(),
            &handle
        );

        auto logger = spdlog::get("hvlib_logger");
        if (result != CAENHV_OK)
        {
            std::string hvlib_error = CAENHV_GetError(handle);
            logger->error("Unable to initialize system. Error: {}", hvlib_error);
        }
        else
        {
            logger->debug("System initialized. Handle: {}", handle);
        }

        return handle;
}

void deinitialize_system(int handle)
{
    CAENHVRESULT result = CAENHV_DeinitSystem(handle);

    auto logger = spdlog::get("hvlib_logger");
    if (result != CAENHV_OK)
    {
        std::string hvlib_error = CAENHV_GetError(handle);
        logger->error("Unable to deinitialize system. Error: {}", hvlib_error);
    }
    else
    {
        logger->debug("Parameter de-initialized");
    }
}

std::tuple<int, int, int, std::string> get_crate_map(int handle)
{
    auto logger = spdlog::get("hvlib_logger");

    unsigned short number_of_slots = -1;
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
    if ((number_of_slots == -1)
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

        deinitialize_system(handle);
        logger->error("{}", message);

        return {256, 256, 256, "ERROR"};
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

    return { number_of_channels, serial_number, slot, firmware_version };
}

template <typename T>
void set_channel_parameter(T val, int handle, std::string parameter, unsigned short channel_number)
{
    unsigned short slot = 0;
    const char* parameter_name_casted = (const char*) parameter.data();
    unsigned short number_of_channels = 1;
    T* address_of_parameter = &val;

    // The HVLIB expects a list of channels. We'll go ahead and just use the
    // pointer decay of an array to pass this in.
    unsigned short channel_list[1] = { channel_number };

    CAENHVRESULT result = CAENHV_SetChParam(
        handle,
        slot,
        parameter_name_casted,
        number_of_channels,
        channel_list,
        (void*) address_of_parameter
    );

    auto logger = spdlog::get("hvlib_logger");

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

template <typename T>
T get_channel_parameter(int handle, std::string parameter, unsigned short channel_number)
{
    unsigned short slot = 0;
    const char* parameter_name_casted = (const char*) parameter.data();
    unsigned short number_of_channels = 1;
    unsigned short channel_list[1] = { channel_number };

    T received_value = (T) 256;

    CAENHVRESULT result = CAENHV_GetChParam(
        handle,
        slot,
        parameter_name_casted,
        number_of_channels,
        channel_list,
        (void*) &received_value
    );

    auto logger = spdlog::get("hvlib_logger");
    if (result != CAENHV_OK)
    {
        std::string hvlib_error = CAENHV_GetError(handle);
        logger->error("Unable to get channel parameter: {}", hvlib_error);
        logger->debug("Returning default value: {}", received_value);
    }
    else
    {
        logger->debug("Parameter read as {}", parameter);
    }

    return received_value;
}
#endif // VIRTUALIZE_CONNECTION 