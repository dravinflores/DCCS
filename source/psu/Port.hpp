#pragma once

#include <string>

namespace msu_smdt 
{
    struct Port 
    {
        std::string_view port { "" };
        std::string_view baud_rate { "" };
        std::string_view data_bit { "" };
        std::string_view stop_bit { "" };
        std::string_view parity { "" };
        std::string_view lbusaddress { "" };
    };
}