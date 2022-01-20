/*******************************************************************************
 *  File:           psu/psu.cpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   17 December, 2021
 * 
 *  Purpose:        This file defines all the functions prototyped in
 *                  the psu.hpp header file.
 * 
 *  Known Issues:   
 *      -   Not using an initializer list for the channel class. This is because
 *          we cannot create a channel object without first creating a
 *          PSUOBJ. Information is needed from the PSUOBJ in order to
 *          initialize the channel object. More study and observation is
 *          required to see if there is a fix.
 * 
 *  Workarounds:    
 *      -   Simply use a default constructor for the channel object, and then
 *          copy by value.
 * 
 *  Updates:
 ******************************************************************************/

#include <psu/psu.hpp>

namespace msu_smdt
{
    // The purpose of this constructor is to initialize the PSUOBJ. Then
    // this constructor will attempt to connect to the power supply, followed
    // by setting all the internal variables to reflect this connection.
    psu::psu(const com_port& com_port_info)
    {
        // We need to check that the proper com_port struct was passed in.
        bool is_empty = com_port_info.port.empty()              \
            || com_port_info.baud_rate.empty()                  \
            || com_port_info.data_bit.empty()                   \
            || com_port_info.stop_bit.empty()                   \
            || com_port_info.parity.empty()                     \
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
        fmt::print("We want to see if we can link to the ");
        fmt::print("CAENHVWrapper library.\n");
    #endif // NDEBUG

        // Here are some variables specific to the power supply initialization.
        this->handle = -1;
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

        // The documentation for the HV Wrapper Library specifically mentions
        // calling this function DIRECTLY AFTER initialization.
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

            // If we have an invalid allocation, we need to 
            // iterate through and free all the rest of the 
            // memory. Then, we will alert the caller. 
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
                message = "Invalid allocation for arguments to "
                          "CAENHV_GetCrateMap";
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
            "{}.{}", 
            firmware_release_max_list[0], 
            firmware_release_minimum_list[0]
        );

        // No clue how this works internally. 
        CAENHV_Free(number_of_channels_list);
        CAENHV_Free(model_list);
        CAENHV_Free(description_list);
        CAENHV_Free(serial_number_list);
        CAENHV_Free(firmware_release_minimum_list);
        CAENHV_Free(firmware_release_max_list);

        // Alright, now we're assuming that we've connected 
        // to the power supply.

        // Brace initialization is awesome.
        msu_smdt::interchannel common_channel_info {
            .handle         { this->handle },
            .slot           { (unsigned short) this->slot },
            .channel_list   { 0, 1, 2, 3 }
        };

        // From the channel constructor, these channels will 
        // default being off.
        msu_smdt::channel CH0(common_channel_info, 0);
        msu_smdt::channel CH1(common_channel_info, 1);
        msu_smdt::channel CH2(common_channel_info, 2);
        msu_smdt::channel CH3(common_channel_info, 3);
    }

    psu::~psu()
    {
    #ifndef NDEBUG
        print_internal_com_port_string();
    #endif // NDEBUG

        // Because this destructor is guaranteed to no-throw, we're going to
        // simply ignore this result. Even if we cannot deinitialize from the
        // system.
        // TODO: Find a better way to deinitialize. Maybe some hard reset?
        CAENHVRESULT result = CAENHV_DeinitSystem(this->handle);
    }
    
    void psu::start_test(int reserve)
    {
        fmt::print("Test starting.\n");

        // For now, we'll just test the functionality of CH0.
        fmt::print("Testing the connection to CH0\n");

        if (CH0.get_polarity() == polarity::normal)
        {
            fmt::print("\tPolarity: +\n");
        }
        else
        {
            fmt::print("\tPolarity: -\n");
        }
        
        fmt::print("\tCurrent: {}\n", CH0.get_current());

        fmt::print("\tVoltage: {}\n", CH0.read_voltage());

        fmt::print("\tChannel Status: {#B}\n", CH0.get_status());

        fmt::print("Attempting to cycle power\n");

        fmt::print("\tPowering on\n");
        CH0.power_on();
        fmt::print("\t\tStatus: {}\n", CH0.get_status() & status::ON);

        fmt::print("\tPowering off\n");
        CH0.power_off();
        fmt::print("\t\tStatus: {}\n", !(CH0.get_status() & status::ON));

        fmt::print("Finished checking connection\n\n");
    }

    void psu::print_internal_com_port_string()
    {
        fmt::print("{}\n", this->com_port_str);
    }
}