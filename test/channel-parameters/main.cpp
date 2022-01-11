#include <map>
#include <string>
#include <vector>
#include <iostream>

#include <CAENHVWrapper.h>

enum class property_type
{
    Float,
    EnumeratedBoolean,
    BitField,
    Integer,
    CString,
    UnsignedShort,
    Default
};

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

    std::cout << "\n";

    CAENHV_Free(parameter_name_list);

    std::vector<std::string> channel_parameter_names {
        "VSet",
        "VMon",
        "ISet",
        "ImonRange",
        "IMonL",
        "IMonH",
        "MaxV",
        "RUp",
        "RDwn",
        "Trip",
        "PDwn",
        "Polarity",
        "ChStatus",
        "Pw"
    };

    std::vector<std::string> property_units {
        "None",
        "A",
        "V",
        "W",
        "C",
        "Hz",
        "Bar",
        "V/s",
        "s",
        "RPM",
        "counts",
        "bit",
        "aps"
    };

    unsigned long property_value_type;

    for (auto& name : channel_parameter_names)
    {
        // For a particular parameter, get the property and type of property.
        CAENHVRESULT result_a = CAENHV_GetChParamProp(
            handle,
            0,
            0,
            name.data(),
            "Type",
            &property_value_type
        );

        std::cout << "For parameter: " << name << "\n";

        if (property_value_type == PARAM_TYPE_NUMERIC)
        {
            std::cout << "\tType: Float\n";

            double minval = 0.00;
            double maxval = 0.00;
            unsigned short unit = 0;
            short exp = 0;

            CAENHVRESULT result_b = CAENHV_GetChParamProp(
                handle,
                0,
                0,
                name.data(),
                "Minval",
                &minval
            );

            std::cout << "\t\tMinval: " << minval << "\n";

            CAENHVRESULT result_c = CAENHV_GetChParamProp(
                handle,
                0,
                0,
                name.data(),
                "Maxval",
                &maxval
            );

            std::cout << "\t\tMaxval: " << maxval << "\n";

            CAENHVRESULT result_d = CAENHV_GetChParamProp(
                handle,
                0,
                0,
                name.data(),
                "Unit",
                &unit
            );

            CAENHVRESULT result_e = CAENHV_GetChParamProp(
                handle,
                0,
                0,
                name.data(),
                "Exp",
                &exp
            );

            char unit_prefix = '';
            switch (exp)
            {
            case -3:
                unit_prefix = 'm';
                break;
            case -6:
                unit_prefix = 'u';
                break;
            case 3:
                unit_prefix = 'K';
                break;
            case 6:
                unit_prefix = 'M';
                break;
            default:
                unit_prefix = '';
                break;
            }

            std::cout 
                << "\t\tUnit: " 
                << unit_prefix
                << property_units[unit] 
                << "\n";

        }
        else if (property_value_type == PARAM_TYPE_ONOFF)
        {
            std::cout << "\tType: Enumerated Boolean\n";
            char state[30];

            CAENHVRESULT result_f = CAENHV_GetChParamProp(
                handle,
                0,
                0,
                name.data(),
                "Onstate",
                state
            );

            std::cout << "\t\tOnstate: " << state << "\n";

            CAENHVRESULT result_g = CAENHV_GetChParamProp(
                handle,
                0,
                0,
                name.data(),
                "Offstate",
                state
            );

            std::cout << "\t\tOffstate: " << state << "\n";
        }
        else if (property_value_type == PARAM_TYPE_CHSTATUS)
        {
            std::cout << "\tType: Bit Field\n";
            std::cout << "\t\tNo Extra Parameters\n";
        }
        else if (property_value_type == PARAM_TYPE_BDSTATUS)
        {
            std::cout << "\tType: Bit Field\n";
            std::cout << "\t\tNo Extra Parameters\n";
        }
        else if (property_value_type == PARAM_TYPE_BINARY)
        {
            std::cout << "\tType: Integer\n";
            std::cout << "\t\tNo Extra Parameters\n";
        }
        else if (property_value_type == PARAM_TYPE_STRING)
        {
            std::cout << "\tType: C String\n";
            std::cout << "\t\tNo Extra Parameters\n";
        }
        else if (property_value_type == PARAM_TYPE_ENUM)
        {
            std::cout << "\tType: Enumeration\n";
            std::cout << "\t\tExtra Parameters (Not Listed).\n";
        }
        else
        {
            std::cout << "\tUnknown Type\n";
        }
    }


    CAENHVRESULT deinit_result = CAENHV_DeinitSystem(handle);

    std::cout << "\nFinished\n\n";
    return 0;
}