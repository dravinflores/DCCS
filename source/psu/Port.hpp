#pragma once

#include <string>

namespace msu_smdt 
{
    struct Port 
    {
        std::string port { "" };
        std::string baud_rate { "" };
        std::string data_bit { "" };
        std::string stop_bit { "" };
        std::string parity { "" };
        std::string lbusaddress { "" };
    };
}