#include "PSUController.hpp"

#include <chrono>
#include <thread>
#include <string>

PSUController::PSUController(bool logging, std::string path):
    forceClosed { false },
    interface { HVInterface(logging, path) },
    logger { nullptr }
{
    if (logging)
    {
        logger = spdlog::get("hvlib_logger");
    }
}

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

void PSUController::pretendTest()
{
    if (logger)
    {
        logger->debug("----- Initiating Test -----");
        logger->debug("Testing connection to CH0");
    }
    
    powerChannelOn(0);
    std::this_thread::sleep_for(std::chrono::seconds(5));

    auto voltage = readVoltage(0);
    auto current = readLowPrecisionCurrent(0);

    if (logger)
    {
        logger->debug("Voltage is: {}", voltage);
        logger->debug("Current is: {}", current);
    }

    powerChannelOff(0);

    if (logger)
        logger->debug("----- Test Completed -----");
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
