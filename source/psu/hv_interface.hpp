/***********************************************************************************************************************
 *  File:           psu/hv_interface.cpp
 *  Author:         Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   27 May 2022
 * 
 *  Purpose:        This header file defines the hv_interface object, which is
 *                  the principle method of interfacing with the HVLIB library
 *                  functions. This library is not the interface with the power
 *                  supply itself, as that interface resides in the psu object.
 * 
 *  Known Issues:       
 * 
 *  Workarounds:
 * 
 *  Updates:
 **********************************************************************************************************************/

#pragma once

#include <string>
#include <memory>

#include <spdlog/spdlog.h>

#include <psu/com_port_struct.hpp>

class hv_interface
{
public:
    hv_interface();
    ~hv_interface();

    hv_interface(hv_interface&) = delete; 
    hv_interface(hv_interface&&) = delete; 

    hv_interface& operator=(hv_interface&) = delete; 
    hv_interface& operator=(hv_interface&&) = delete; 

    void connect(const msu_smdt::com_port& port);
    void disconnect();

    bool is_connected_to_power_supply();
    // unsigned short get_serial_number();
    // std::string get_firmware();

    void set_parameter_float(float val, std::string parameter, unsigned short channel);
    void set_parameter_long(unsigned long val, std::string parameter, unsigned short channel);
    
    float get_parameter_float(std::string parameter, unsigned short channel);
    unsigned long get_parameter_long(std::string parameter, unsigned short channel);

private:
    int handle;
    bool connection_status;
    std::shared_ptr<spdlog::logger> logger;

    // The following members and functions are used internally.

    enum class type
    {
        FLOAT,
        UNSIGNED
    };

    struct value
    {
        float f;
        unsigned long l;
    };

    void get_crate_map();
    void set_parameter_primitive(value new_val, type type, std::string parameter, unsigned short channel);
    value get_parameter_primitive(type type, std::string parameter, unsigned short channel);
};