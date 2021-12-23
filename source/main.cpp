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
    // fmt::print("Hello, World!\n");
    fmt::print("\nPrinting arguments passed in to main.\n");

    for (int i = 0; i < argc; ++i)
    {
        fmt::print("\tArg {}: {}\n", i, argv[i]);
    }

    fmt::print("We will assume that Arg 1 being passed in means specifying ");
    fmt::print("the COM Port.\n\n");

    char* com_port;

    if (argc > 1)
    {
        com_port = argv[1];
    }
    else
    {
        com_port = "COM4";
    }

    msu_smdt::com_port fake_com_port_connection {
        /* port */          com_port,
        /* baud_rate */     "9600",
        /* data_bit */      "8",
        /* stop_bit */      "0",
        /* parity */        "0",
        /* lbusaddress */   "0"
    };

    msu_smdt::psu psuobj(fake_com_port_connection);

    return 0;
}