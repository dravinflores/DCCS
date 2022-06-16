#pragma once

#include <string>
#include <memory>
#include <stdexcept>

#include <spdlog/spdlog.h>

#include "Port.hpp"

struct PowerSupplyProperties
{
    std::string Board;
    std::string Model;
    std::string Description;
    std::string NumberOfSlots;
    std::string ChannelsAvailable;
    std::string Serial;
    std::string Firmware;
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

    void connect(msu_smdt::Port port);
    void disconnect();
    bool isConnected();

    PowerSupplyProperties getProperties();

    void setParameterFloat(float value, std::string parameter, int channel);
    void setParameterInt(int value, std::string parameter, int channel);

    float getParameterFloat(std::string parameter, int channel);
    int getParameterInt(std::string parameter, int channel);

private:
    int handle;
    bool connected;
    PowerSupplyProperties properties;
    std::shared_ptr<spdlog::logger> logger;
};