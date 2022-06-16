#include "PSUController.hpp"

#include <chrono>
#include <thread>
#include <string>

#include <spdlog/sinks/stdout_color_sinks.h>

PSUController::PSUController():
    forceClosed { false },
    interface(),
    logger { spdlog::stdout_color_mt("PSUController") }
{}

PSUController::~PSUController()
{}

void PSUController::connect(msu_smdt::Port port)
{
    try
    {
        interface.connect(port);
    }
    catch (std::runtime_error& error)
    {
        if (logger)
        {
            logger->error("PSUController cannot connect to HVInterface");
        }
        throw;
    }
}

void PSUController::disconnect()
{
    try
    {
        interface.disconnect();
    }
    catch (std::runtime_error& error)
    {
        if (logger)
        {
            logger->error("PSUController cannot disconnect to HVInterface");
        }
        throw;
    }
}

void PSUController::prepareChannelsForTesting(std::vector<int> channels_to_activate)
{
    for (auto& channel : channels_to_activate)
    {
        // Set defaults.
        setTestVoltage(channel, 15.00f);
        setCurrentLimit(channel, 2.00f);
        setMaxVoltage(channel, 4015.00f);
        setRampUpRate(channel, 15.00f);
        setRampDownRate(channel, 15.00f);
        setOverCurrentTimeLimit(channel, 1000.00f);
        setPowerDownMethod(channel, RecoveryMethod::Ramp);
    }
}

void PSUController::powerChannelOn(int channel)
{
    unsigned long value = 1;
    interface.setParameterInt(value, "Pw", channel);
}

void PSUController::powerChannelOff(int channel)
{
    unsigned long value = 0;
    interface.setParameterInt(value, "Pw", channel);
}

float PSUController::getTestVoltage(int channel)
{
    return interface.getParameterFloat("VSet", channel);
}

void PSUController::setTestVoltage(int channel, float value)
{
    interface.setParameterFloat(value, "VSet", channel);
}

void PSUController::setMaxVoltage(int channel, float value)
{
    interface.setParameterFloat(value, "MaxV", channel);
}

void PSUController::setCurrentLimit(int channel, float value)
{
    interface.setParameterFloat(value, "ISet", channel);
}

void PSUController::setOverCurrentTimeLimit(int channel, float value)
{
    interface.setParameterFloat(value, "Trip", channel);
}

void PSUController::setRampUpRate(int channel, float value)
{
    interface.setParameterFloat(value, "RUp", channel);
}

void PSUController::setRampDownRate(int channel, float value)
{
    interface.setParameterFloat(value, "RDwn", channel);
}

void PSUController::setPowerDownMethod(int channel, RecoveryMethod method)
{
    unsigned long value = 1;

    if (method == RecoveryMethod::Kill)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }

    interface.setParameterInt(value, "PDwn", channel);
}

float PSUController::readVoltage(int channel)
{
    return interface.getParameterFloat("VMon", channel);
}

bool PSUController::readPolarity(int channel)
{
    return !(interface.getParameterInt("Polarity", channel));
}

bool PSUController::isInHighPrecision(int channel)
{
    return !(interface.getParameterInt("ImonRange", channel));
}

float PSUController::readLowPrecisionCurrent(int channel)
{
    return interface.getParameterFloat("IMonL", channel);
}

float PSUController::readHighPrecisionCurrent(int channel)
{
    return interface.getParameterFloat("IMonH", channel);
}

unsigned long PSUController::readChannelStatus(int channel)
{
    return interface.getParameterInt("ChStatus", channel);
}
