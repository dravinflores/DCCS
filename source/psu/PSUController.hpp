#pragma once

#include <vector>

#include <spdlog/spdlog.h>

#include "HVInterface.hpp"
#include "Port.hpp"

enum class RecoveryMethod
{
    Kill,
    Ramp
};

class PSUController
{
public:
    PSUController();
    ~PSUController();

    PSUController(const PSUController&) = delete;
    PSUController(PSUController&&) = delete;

    PSUController& operator=(const PSUController&) = delete;
    PSUController& operator=(PSUController&&) = delete;

    void connect(msu_smdt::Port port);
    void disconnect();

    void prepareChannelsForTesting(std::vector<int> channels_to_activate);

    void powerChannelOn(int channel);
    void powerChannelOff(int channel);

    float getTestVoltage(int channel);

    void setTestVoltage(int channel, float value);
    void setMaxVoltage(int channel, float value);

    void setCurrentLimit(int channel, float value);
    void setOverCurrentTimeLimit(int channel, float value);

    void setRampUpRate(int channel, float value);
    void setRampDownRate(int channel, float value);
    void setPowerDownMethod(int channel, RecoveryMethod method);

    float readVoltage(int channel);
    
    bool readPolarity(int channel);
    bool isInHighPrecision(int channel);

    float readLowPrecisionCurrent(int channel);
    float readHighPrecisionCurrent(int channel);

    unsigned long readChannelStatus(int channel);

private:
    bool forceClosed;
    HVInterface interface;
    std::shared_ptr<spdlog::logger> logger;
};