#include <exception>

#include <spdlog/sinks/stdout_color_sinks.h>

#include "PSUController.hpp"

using CHVector = std::vector<unsigned short>;

constexpr float VSetMax = 5500.0f;
constexpr float ISetMax = 21.000f;
constexpr float MaxVMax = 5600.0f;
constexpr float RUpMax  = 500.00f;
constexpr float RDwnMax = 500.00f;
constexpr float TripMax = 1000.0f;

constexpr float epsilon = 0.001f;

PSUController::PSUController():
    forceClosed { false }
{
    try
    {
        logger = spdlog::stdout_color_mt("PSULogger");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        logger = spdlog::get("PSULogger");
    }
}

PSUController::~PSUController()
{}

void PSUController::connectToPSU(msu_smdt::Port port)
{
    try
    {
        interface.connectToPSU(port);
        interface.clearAlarm();
        interface.setInterlock(true);
    }
    catch (const std::exception& exception)
    {
        logger->error("PSUController cannot connect to Power Supply");
        throw;
    }
}

void PSUController::disconnectFromPSU()
{
    try
    {
        interface.disconnectFromPSU();
    }
    catch (const std::exception& exception)
    {
        logger->error("PSUController cannot disconnect from Power Supply");
        throw;
    }
}

void PSUController::powerOnChannels(CHVector channels)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        interface.setParametersLong("Pw", 1, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

void PSUController::powerOffChannels(CHVector channels)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        interface.setParametersLong("Pw", 0, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

std::vector<float> PSUController::getTestVoltages(CHVector channels)
{
    std::vector<float> returnVector(channels.size(), -1.00f);

    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        returnVector = interface.getParametersFloat("VSet", channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }

    return returnVector;
}

void PSUController::setTestVoltages(CHVector channels, float voltage)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    if (voltage < 0.00f - epsilon || voltage > VSetMax + epsilon)
    {
        logger->error("Voltage value out of range");
        throw std::runtime_error("Voltage value out of range");
    }

    try
    {
        interface.setParametersFloat("VSet", voltage, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

void PSUController::setTestCurrents(CHVector channels, float current)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    if (current < 0.00f - epsilon || current > ISetMax + epsilon)
    {
        logger->error("Current out of range");
        throw std::runtime_error("Current out of range");
    }

    try
    {
        interface.setParametersFloat("ISet", current, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

void PSUController::setMaxVoltages(CHVector channels, float voltage)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    if (voltage < 0.00f - epsilon || voltage > MaxVMax + epsilon)
    {
        logger->error("Voltage value out of range");
        throw std::runtime_error("Voltage value out of range");
    }

    try
    {
        interface.setParametersFloat("MaxV", voltage, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

void PSUController::setOverCurrentLimits(CHVector channels, float time)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    if (time < 0.00f - epsilon || time > TripMax + epsilon)
    {
        logger->error("Over Current Time value out of range");
        throw std::runtime_error("Over Current Time value out of range");
    }

    try
    {
        interface.setParametersFloat("Trip", time, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

void PSUController::setRampUpRate(CHVector channels, float rate)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    if (rate < 1.00f - epsilon || rate > RUpMax + epsilon)
    {
        logger->error("Ramp Up Rate out of range");
        throw std::runtime_error("Ramp Up Rate out of range");
    }

    try
    {
        interface.setParametersFloat("RUp", rate, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

void PSUController::setRampDownRate(CHVector channels, float rate)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    if (rate < 1.00f - epsilon || rate > RDwnMax + epsilon)
    {
        logger->error("Ramp Down Rate out of range");
        throw std::runtime_error("Ramp Down Rate out of range");
    }

    try
    {
        interface.setParametersFloat("RDwn", rate, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

void PSUController::killChannelsAfterTest(CHVector channels, bool kill)
{
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        unsigned long value = 1;

        if (kill)
            value = 0;

        interface.setParametersLong("PDwn", value, channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }
}

std::vector<float> PSUController::readVoltages(CHVector channels)
{
    std::vector<float> returnVector(channels.size(), -1.00f);
    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        returnVector = interface.getParametersFloat("VMon", channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }

    return returnVector;
}

std::vector<float> PSUController::readCurrents(CHVector channels)
{
    std::vector<float> returnVector(channels.size(), -1.00f);

    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        returnVector = interface.getParametersFloat("IMonH", channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }

    return returnVector;
}

std::vector<unsigned long> PSUController::readPolarities(CHVector channels)
{
    std::vector<unsigned long> returnVector(channels.size(), false);

    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        returnVector = interface.getParametersLong("Polarity", channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }

    return returnVector;
}

std::vector<unsigned long> PSUController::readStatuses(CHVector channels)
{
    std::vector<unsigned long> returnVector(channels.size(), 0);

    if (channels.size() > 4)
    {
        logger->error("Too many channels");
        throw std::runtime_error("Too many channels. Expected 4 channels or less.");
    }

    try
    {
        returnVector = interface.getParametersLong("ChStatus", channels);
    }
    catch (const std::exception& exception)
    {
        logger->error("Caught exception in PSUController: {}", exception.what());
        throw;
    }

    return returnVector;
}
