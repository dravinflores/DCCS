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
    fmt::print("Hello, World!\n");

    msu_smdt::com_port fake_com_port_connection {
        /* port */          "COM1",
        /* baud_rate */     "9600",
        /* data_bit */      "8",
        /* stop_bit */      "1",
        /* parity */        "None",
        /* lbusaddress */   "0"
    };

    msu_smdt::psu psuobj(fake_com_port_connection);

    return 0;
}