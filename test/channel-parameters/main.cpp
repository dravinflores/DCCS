#include <string>
#include <iostream>

#include <CAENHVWrapper.h>

int main(int argc, char** argv)
{
    std::cout << "Testing Channel Parameter Names\n\n";

    std::string com_port_str = "COM4_9600_8_0_0_0";
    int handle = -1;
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

    unsigned short  number_of_slots = -1;
    unsigned short* number_of_channels_list;
    char*           model_list;
    char*           description_list;
    unsigned short* serial_number_list;
    unsigned char*  firmware_release_minimum_list;
    unsigned char*  firmware_release_max_list;

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

    CAENHV_Free(number_of_channels_list);
    CAENHV_Free(model_list);
    CAENHV_Free(description_list);
    CAENHV_Free(serial_number_list);
    CAENHV_Free(firmware_release_minimum_list);
    CAENHV_Free(firmware_release_max_list);

    char* parameter_name_list;
    int number_of_parameters;

    CAENHVRESULT ch_result = CAENHV_GetChParamInfo(
        handle,
        0,
        0,
        &parameter_name_list,
        &number_of_parameters
    );

    char (*parameter)[10] = ( char(*)[10] )parameter_name_list;

    for (int i = 0; i < number_of_parameters; ++i)
    {
        std::cout << "\tParameter " << i << ": " << parameter[i] << "\n";
    }

    CAENHV_Free(parameter_name_list);

    CAENHVRESULT deinit_result = CAENHV_DeinitSystem(handle);

    std::cout << "\nFinished\n\n";
    return 0;
}