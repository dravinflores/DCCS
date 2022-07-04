#pragma once

#include <string>
#include <memory>
#include <stdexcept>

#include <spdlog/spdlog.h>

#include "Port.hpp"

struct PowerSupplyProperties
{
    std::string Board               { "N/A" };
    std::string Model               { "N/A" };
    std::string Description         { "N/A" };
    std::string NumberOfSlots       { "N/A" };
    std::string ChannelsAvailable   { "N/A" };
    std::string Serial              { "N/A" };
    std::string Firmware            { "N/A" };
};

class HVInterface 
{
public:
    HVInterface();
    ~HVInterface();

    HVInterface(const HVInterface&) = delete;
    HVInterface(HVInterface&&) = delete;

    HVInterface& operator=(const HVInterface&) = delete;
    HVInterface& operator=(HVInterface&&) = delete;

    void connectToPSU(msu_smdt::Port port);
    void disconnectFromPSU();
    bool isConnectedToPSU();

    PowerSupplyProperties getProperties();

    void setParametersFloat(std::string parameter, float value, std::vector<unsigned short> channels);
    void setParametersLong(std::string parameter, unsigned long value, std::vector<unsigned short> channels);
    
    std::vector<float> getParametersFloat(std::string parameter, std::vector<unsigned short> channels);
    std::vector<unsigned long> getParametersLong(std::string parameter, std::vector<unsigned short> channels);

    bool checkAlarm();
    bool checkInterlock();

    void clearAlarm();
    void setInterlock(bool state);

private:
    int handle;
    bool connected;
    PowerSupplyProperties properties;
    std::shared_ptr<spdlog::logger> logger;
};