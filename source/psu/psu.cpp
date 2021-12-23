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

    // Here we have some debugging information.
    fmt::print(
        "Here in the file {} on line {}, is_empty = {}.\n", 
        __FILE__, 
        __LINE__, 
        is_empty
    );

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

    // fmt::print("COM_PORT_STR = {}\n", this->com_port_str);

    fmt::print("We want to see if we can link to the CAENHVWrapper library.\n");

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

    fmt::print("Attempting connection. Result is {}.\n", result);

    // Made to be more explicit.
    if (result != CAENHV_OK)
    {    
        std::string_view error = CAENHV_GetError(this->handle);
        fmt::print("\tAttempting to read error: {}\n", error);

    }
    else
    {
        // Would it be more efficient to just not have this else
        // case, and have some mechanism to escape the
        // constructor if the result isn't CAENHV_OK?
        // TODO: Consider this.
        int temp = 0;

        // TODO: Need a better system. This is kinda clunky;
        unsigned short number_of_slots;
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

        // We might have the potential for a memory leak here.
        if (crate_map_result != CAENHV_OK)
        {
            std::string_view error = CAENHV_GetError(this->handle);
            fmt::print("\tAttempting to read error: {}\n", error);
        }
        else
        {
            bool invalid_allocation = number_of_channels_list == nullptr       \
                || model_list == nullptr                                       \
                || description_list == nullptr                                 \
                || serial_number_list == nullptr                               \
                || firmware_release_minimum_list == nullptr                    \
                || firmware_release_max_list == nullptr;
            // We should check for a nullptr here.
            if (invalid_allocation)
            {
                // We should probably figure out which is invalid. However,
                // this case should ideally never be called. The CAEN HV Wrapper
                // Library documentation should set the empty pointers to
                // empty strings? Can't tell.
                // TODO: Investigate.
                fmt::print("\n\n\t\t Invalid Allocation!\n\n");
            }

            // Oh great, we're going to have to iterate through these
            // dynamically allocated lists/strings.
            bool is_end_of_list = false;
            int i = 0;

            // Model list is literally a large string, but with different
            // sub-strings mixed in. Likewise with the description.
            char* model = model_list;
            char* description = description_list;

            while (!is_end_of_list)
            {
                if (*model == '\n')
                {
                    fmt::print("No more models available\n");
                }
                else
                {
                    fmt::print("\tBoard #{}\n", i);
                    fmt::print("\tModel: {}\n", model_list);
                    fmt::print("\tDescription: {}\n", description_list);
                    fmt::print(
                        "\tChannels available: {}\n", number_of_channels_list[i]
                    );
                    fmt::print("\tSerial: {}\n", serial_number_list[i]);
                    fmt::print(
                        "\tFirmware: {}.{}", 
                        firmware_release_max_list[i],
                        firmware_release_minimum_list[i]
                    );
                }

                ++i;

                if (i < number_of_slots)
                {
                    model += std::strlen(model);
                    description += std::strlen(description_list);
                }
                else
                {
                    is_end_of_list = true;
                    fmt::print("\n\nNo more information available");
                }
            }

            // No clue how this works internally. 
            CAENHV_Free(number_of_channels_list);
            CAENHV_Free(model_list);
            CAENHV_Free(description_list);
            CAENHV_Free(serial_number_list);
            CAENHV_Free(firmware_release_minimum_list);
            CAENHV_Free(firmware_release_max_list);
        }
    }
}

/*
msu_smdt::psu& msu_smdt::psu::operator=(msu_smdt::psu&& other_psu_obj) noexcept
{}

msu_smdt::psu::psu(msu_smdt::psu&& other_psu_obj) noexcept
{}

*/
msu_smdt::psu::~psu()
{
    print_internal_com_port_string();
    CAENHVRESULT result = CAENHV_DeinitSystem(this->handle);
    fmt::print("\n");
}


void msu_smdt::psu::print_internal_com_port_string()
{
    fmt::print("{}\n", this->com_port_str);
}