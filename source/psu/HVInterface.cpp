#include "HVInterface.hpp"

#include <exception>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef VIRTUALIZE_CONNECTION
#include "FakeHV/FakeHVLibrary.h"
#else
#include <CAENHVWrapper.h>
#endif

using SpdlogLogger = std::shared_ptr<spdlog::logger>;

static constexpr int default_value { 512 };

HVInterface::HVInterface():
    handle { -1 },
    connected { false },
    logger { spdlog::stdout_color_mt("HVInterface") }
{}

HVInterface::~HVInterface()
{
    // Auto-disconnect for convenience.
    if (connected)
    {
        disconnect();
    }
}

static PowerSupplyProperties get_crate_map(SpdlogLogger& logger, int handle)
{
    // Allocate according to the library specifications
    unsigned short numberOfSlots;
    unsigned short* listOfChannelsIndexedBySlot = nullptr;
    char* listOfModelsIndexedBySlot = nullptr;
    char* descriptionList = nullptr;
    unsigned short* listOfSerialNumbersIndexedBySlot = nullptr;
    unsigned char* listOfFirmwareSuffixesIndexedBySlot = nullptr;
    unsigned char* listOfFirmwarePrefixesIndexedBySlot = nullptr;

    PowerSupplyProperties properties;

#ifdef VIRTUALIZE_CONNECTION
    int result = FakeHV_GetCrateMap(
        handle,
        &numberOfSlots,
        &listOfChannelsIndexedBySlot,
        &listOfModelsIndexedBySlot,
        &descriptionList,
        &listOfSerialNumbersIndexedBySlot,
        &listOfFirmwareSuffixesIndexedBySlot,
        &listOfFirmwarePrefixesIndexedBySlot
    );
#else
    CAENHVRESULT result = CAENHV_GetCrateMap(
        handle,
        &numberOfSlots,
        &listOfChannelsIndexedBySlot,
        &listOfModelsIndexedBySlot,
        &descriptionList,
        &listOfSerialNumbersIndexedBySlot,
        &listOfFirmwareSuffixesIndexedBySlot,
        &listOfFirmwarePrefixesIndexedBySlot
    );
#endif

    if (result != 0)
    {
    #ifdef VIRTUALIZE_CONNECTION
        auto message = fmt::format("FakeHV_GetCrateMap {}", FakeHV_GetError(handle));
    #else
        auto message = fmt::format("CAENHV_GetCrateMap Error: {}", CAENHV_GetError(handle));
    #endif

        if (logger)
        {
            logger->error(message);
        }

        if (listOfChannelsIndexedBySlot)
        {
        #ifdef VIRTUALIZE_CONNECTION
            FakeHV_Free(listOfChannelsIndexedBySlot);
        #else
            CAENHV_Free(listOfChannelsIndexedBySlot);
        #endif 
        }
        if (listOfModelsIndexedBySlot)
        {
        #ifdef VIRTUALIZE_CONNECTION
            FakeHV_Free(listOfModelsIndexedBySlot);
        #else
            CAENHV_Free(listOfModelsIndexedBySlot);
        #endif 
        }
        if (descriptionList)
        {
        #ifdef VIRTUALIZE_CONNECTION
            FakeHV_Free(descriptionList);
        #else
            CAENHV_Free(descriptionList);
        #endif 
        }
        if (listOfSerialNumbersIndexedBySlot)
        {
        #ifdef VIRTUALIZE_CONNECTION
            FakeHV_Free(listOfSerialNumbersIndexedBySlot);
        #else
            CAENHV_Free(listOfSerialNumbersIndexedBySlot);
        #endif 
        }
        if (listOfFirmwareSuffixesIndexedBySlot)
        {
        #ifdef VIRTUALIZE_CONNECTION
            FakeHV_Free(listOfFirmwareSuffixesIndexedBySlot);
        #else
            CAENHV_Free(listOfFirmwareSuffixesIndexedBySlot);
        #endif 
        }
        if (listOfFirmwarePrefixesIndexedBySlot)
        {
        #ifdef VIRTUALIZE_CONNECTION
            FakeHV_Free(listOfFirmwarePrefixesIndexedBySlot);
        #else
            CAENHV_Free(listOfFirmwarePrefixesIndexedBySlot);
        #endif 
        }

        // We'll throw to alert the caller.
        throw std::runtime_error(message);
    }
    else
    {
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
        properties.Board = fmt::format("{}", 0);
        properties.Model = fmt::format("{}", listOfModelsIndexedBySlot[0]);
        properties.Description = fmt::format("{}", descriptionList[0]);
        properties.NumberOfSlots = fmt::format("{}", numberOfSlots);
        properties.ChannelsAvailable = fmt::format("{}", listOfChannelsIndexedBySlot[0]);
        properties.Serial = fmt::format("{}", listOfSerialNumbersIndexedBySlot[0]);
        properties.Firmware = fmt::format(
            "{}.{}", 
            listOfFirmwarePrefixesIndexedBySlot[0], 
            listOfFirmwareSuffixesIndexedBySlot[0]
        );

    #ifdef VIRTUALIZE_CONNECTION
        FakeHV_Free(listOfChannelsIndexedBySlot);
        FakeHV_Free(listOfModelsIndexedBySlot);
        FakeHV_Free(descriptionList);
        FakeHV_Free(listOfSerialNumbersIndexedBySlot);
        FakeHV_Free(listOfFirmwareSuffixesIndexedBySlot);
        FakeHV_Free(listOfFirmwarePrefixesIndexedBySlot);
    #else
        CAENHV_Free(listOfChannelsIndexedBySlot);
        CAENHV_Free(listOfModelsIndexedBySlot);
        CAENHV_Free(descriptionList);
        CAENHV_Free(listOfSerialNumbersIndexedBySlot);
        CAENHV_Free(listOfFirmwareSuffixesIndexedBySlot);
        CAENHV_Free(listOfFirmwarePrefixesIndexedBySlot);
    #endif 

        if (logger)
        {
            logger->debug("Get crate map was successful.");
        }
    }

    return properties;
}

template <typename T>
static void setParameter(T val, std::string parameter, int channel, SpdlogLogger& logger, int handle)
{
    unsigned short slot = 0;
    unsigned short channelListSize = 1;
    unsigned short listOfChannelsToWrite[] = { (unsigned short) channel };
    T newParameterValue;

#ifdef VIRTUALIZE_CONNECTION
    int result = FakeHV_SetChannelParameter(
        handle,
        slot,
        (const char*) parameter.c_str(),
        channelListSize,
        listOfChannelsToWrite,
        (void*) &newParameterValue
    );
#else
    CAENHVRESULT result = CAENHV_SetChParam(
        handle,
        slot,
        (const char*) parameter.c_str(),
        channelListSize,
        listOfChannelsToWrite,
        (void*) &newParameterValue
    );
#endif

    if (result != 0)
    {
    #ifdef VIRTUALIZE_CONNECTION
        auto message = fmt::format("Unable to set parameter {}. {}", parameter, FakeHV_GetError(handle));
    #else
        auto message = fmt::format("Unable to set parameter {}. Error: {}", parameter, CAENHV_GetError(handle));
    #endif

        if (logger)
        {
            logger->error(message);
        }

        throw std::runtime_error(message);
    }
    else
    {
        auto message = fmt::format("Parameter {} was set successful.", parameter);
        
        if (logger)
        {
            logger->debug(message);
        }
    }
}

template <typename T>
static T getParameter(T hint, std::string parameter, int channel, SpdlogLogger& logger, int handle)
{
    unsigned short slot = 0;
    unsigned short channelListSize = 1;
    unsigned short listOfChannelsToRead[] = { (unsigned short) channel };
    T listOfParameterValues[1];

#ifdef VIRTUALIZE_CONNECTION
    int result = FakeHV_GetChannelParameter(
        handle,
        slot,
        (const char*) parameter.c_str(),
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );
#else
    int result = CAENHV_GetChParam(
        handle,
        slot,
        (const char*) parameter.c_str(),
        channelListSize,
        listOfChannelsToRead,
        (void*) listOfParameterValues
    );
#endif

    if (result != 0)
    {
    #ifdef VIRTUALIZE_CONNECTION
        auto message = fmt::format("Unable to get parameter {}. {}", parameter, FakeHV_GetError(handle));
    #else
        auto message = fmt::format("Unable to get parameter {}. Error: {}", parameter, CAENHV_GetError(handle));
    #endif

        if (logger)
        {
            logger->error(message);
        }

        throw std::runtime_error(message);
    }
    else
    {
        auto message = fmt::format(
            "Parameter {} successfully retrieved. Value: {}", 
            parameter,
            (T) listOfParameterValues[0]
        );
        
        if (logger)
        {
            logger->debug(message);
        }
    }

    return (T) listOfParameterValues[0];
}

void HVInterface::connect(msu_smdt::Port port)
{
    int system = 6;
    int linkType = 5;
    auto connectionString = fmt::format(
        "{}_{}_{}_{}_{}_{}", 
        port.port,
        port.baud_rate, 
        port.data_bit,
        port.stop_bit,
        port.parity,
        port.lbusaddress
    );

    const char* username = "";
    const char* password = "";
    int handle;

#ifdef VIRTUALIZE_CONNECTION
    int result = FakeHV_InitializeSystem(
        system,
        linkType,
        (void*) connectionString.c_str(),
        username,
        password,
        &handle
    );
#else
    int result = CAENHV_InitSystem(
        system,
        linkType,
        (void*) connectionString.c_str(),
        username,
        password,
        &handle
    );
#endif

    if (result != 0)
    {
    #ifdef VIRTUALIZE_CONNECTION
        auto message = fmt::format("Unable to connect. {}", FakeHV_GetError(handle));
    #else
        auto message = fmt::format("Unable to connect. Error: {}", CAENHV_GetError(handle));
    #endif

        if (logger)
        {
            logger->error(message);
        }

        throw std::runtime_error(message);
    }
    else
    {
        this->handle = handle;
        this->connected = true;
        auto message = fmt::format("Connection was succesful.");
        
        if (logger)
        {
            logger->debug(message);
        }
    }
}

void HVInterface::disconnect()
{
#ifdef VIRTUALIZE_CONNECTION
    int result = FakeHV_DeinitializeSystem(this->handle);
#else
    int result = CAENHV_DeinitSystem(this->handle);
#endif

    if (result != 0)
    {
    #ifdef VIRTUALIZE_CONNECTION
        auto message = fmt::format("Unable to disconnect. {}", FakeHV_GetError(handle));
    #else
        auto message = fmt::format("Unable to disconnect. Error: {}", CAENHV_GetError(handle));
    #endif

        if (logger)
        {
            logger->error(message);
        }

        throw std::runtime_error(message);
    }
    else
    {
        this->handle = -1;
        this->connected = false;
        auto message = fmt::format("Successfully Disconnected.");
        
        if (logger)
        {
            logger->debug(message);
        }
    }
}

bool HVInterface::isConnected()
{
    return this->connected;
}

PowerSupplyProperties HVInterface::getProperties()
{
    return this->properties;
}

void HVInterface::setParameterFloat(float value, std::string parameter, int channel)
{
    setParameter(value, parameter, channel, this->logger, this->handle);
}

void HVInterface::setParameterInt(int value, std::string parameter, int channel)
{
    setParameter(value, parameter, channel, this->logger, this->handle);
}

float HVInterface::getParameterFloat(std::string parameter, int channel)
{
    float hint = 0.00f;
    return getParameter(hint, parameter, channel, this->logger, this->handle);
}

int HVInterface::getParameterInt(std::string parameter, int channel)
{
    int hint = 0;
    return getParameter(hint, parameter, channel, this->logger, this->handle);
}
