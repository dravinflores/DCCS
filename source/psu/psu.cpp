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
    ).data();

    fmt::print("We want to see if we can link to the CAENHVWrapper library.\n");

    int handle = -1;
    CAENHV_SYSTEM_TYPE_t system = N1470;
    int link_type = LINKTYPE_USB_VCP;
    const char* arg = "";
    const char* username = "";
    const char* password = "";

    CAENHVRESULT result = CAENHV_InitSystem(
        system, 
        link_type, 
        (void*) arg, 
        username, 
        password,
        &handle
    );

    fmt::print("Attempting connection. Result is {}.\n", result);
}

/*
msu_smdt::psu& msu_smdt::psu::operator=(msu_smdt::psu&& other_psu_obj) noexcept
{}

msu_smdt::psu::psu(msu_smdt::psu&& other_psu_obj) noexcept
{}

*/
msu_smdt::psu::~psu()
{
    fmt::print("Destructor called.");
}


void msu_smdt::psu::print_internal_com_port_string()
{
    fmt::print("{}\n", this->com_port_str);
}