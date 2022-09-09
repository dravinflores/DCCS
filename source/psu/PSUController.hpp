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

    void powerOnChannels(std::vector<int> channels);
    void powerOffChannels(std::vector<int> channels);

    std::vector<float> getTestVoltages(std::vector<int> channels);

    void setTestVoltages(std::vector<int> channels, float voltage);
    void setTestCurrents(std::vector<int> channels, float current);

    void setMaxVoltages(std::vector<int> channels, float voltage);
    void setOverCurrentLimits(std::vector<int> channels, float time);

    void setRampUpRate(std::vector<int> channels, float rate);
    void setRampDownRate(std::vector<int> channels, float rate);

    void killChannelsAfterTest(std::vector<int> channels, bool kill);

    std::vector<float> readVoltages(std::vector<int> channels);
    std::vector<float> readCurrents(std::vector<int> channels);
    std::vector<unsigned long> readPolarities(std::vector<int> channels);

    std::vector<unsigned long> readStatuses(std::vector<int> channels);

private:
    bool forceClosed;
    HVInterface interface;
    std::shared_ptr<spdlog::logger> logger;
};