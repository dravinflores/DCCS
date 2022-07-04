#pragma once

#include <vector>
#include <memory>

#include <spdlog/spdlog.h>

#include "HVInterface.hpp"
#include "Port.hpp"

class PSUController
{
public:
    PSUController();
    ~PSUController();

    PSUController(const PSUController&) = delete;
    PSUController(PSUController&&) = delete;

    PSUController& operator=(const PSUController&) = delete;
    PSUController& operator=(PSUController&&) = delete;

    void connectToPSU(msu_smdt::Port port);
    void disconnectFromPSU();

    void powerOnChannels(std::vector<unsigned short> channels);
    void powerOffChannels(std::vector<unsigned short> channels);

    std::vector<float> getTestVoltages(std::vector<unsigned short> channels);

    void setTestVoltages(std::vector<unsigned short> channels, float voltage);
    void setTestCurrents(std::vector<unsigned short> channels, float current);

    void setMaxVoltages(std::vector<unsigned short> channels, float voltage);
    void setOverCurrentLimits(std::vector<unsigned short> channels, float time);

    void setRampUpRate(std::vector<unsigned short> channels, float rate);
    void setRampDownRate(std::vector<unsigned short> channels, float rate);

    void killChannelsAfterTest(std::vector<unsigned short> channels, bool kill);

    std::vector<float> readVoltages(std::vector<unsigned short> channels);
    std::vector<float> readCurrents(std::vector<unsigned short> channels);
    std::vector<unsigned long> readPolarities(std::vector<unsigned short> channels);

    std::vector<unsigned long> readStatuses(std::vector<unsigned short> channels);

private:
    bool forceClosed;
    HVInterface interface;
    std::shared_ptr<spdlog::logger> logger;
};