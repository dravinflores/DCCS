/*******************************************************************************
 *  File:           main.cpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   16 December, 2021
 * 
 *  Purpose:        This is the main entry of the program.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 ******************************************************************************/

#include <fmt/core.h>

#include <psu/psu.hpp>

int main(int argc, char** argv)
{
    fmt::print("\nPrinting arguments passed in to main.\n");

    for (int i = 0; i < argc; ++i)
    {
        fmt::print("\tArg {}: {}\n", i, argv[i]);
    }

    fmt::print("\n");

    msu_smdt::com_port fake_com_port_connection {
        /* port */          "COM4",
        /* baud_rate */     "9600",
        /* data_bit */      "8",
        /* stop_bit */      "0",
        /* parity */        "0",
        /* lbusaddress */   "0"
    };

    try
    {
        msu_smdt::psu psuobj;
        psuobj.initialize(fake_com_port_connection);
        psuobj.start_test(0);
    }
    catch(const std::exception& e)
    {
        fmt::print("Unable to connect: {}\n\n", e.what());
    }

    return 0;
}