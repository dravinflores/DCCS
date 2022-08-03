#include "HVInterface.hpp"

#include <exception>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef VIRTUALIZE_CONNECTION
    #include "FakeHV/FakeHVLibrary.h"
    #define InitializeSystem    FakeHV_InitializeSystem
    #define DeinitializeSystem  FakeHV_DeinitializeSystem
    #define GetCrateMap         FakeHV_GetCrateMap
    #define GetChannelParameter FakeHV_GetChannelParameter
    #define SetChannelParameter FakeHV_SetChannelParameter
    #define GetError            FakeHV_GetError
    #define Free                FakeHV_Free
#else
    #include <CAENHVWrapper.h>
    #define InitializeSystem    CAENHV_InitSystem
    #define DeinitializeSystem  CAENHV_DeinitSystem
    #define GetCrateMap         CAENHV_GetCrateMap
    #define GetChannelParameter CAENHV_GetChParam
    #define SetChannelParameter CAENHV_SetChParam
    #define GetError            CAENHV_GetError
    #define Free                CAENHV_Free
#endif

using CHVector = std::vector<int>;
using FloatVector = std::vector<float>;
using ULongVector = std::vector<unsigned long>;
using SpdlogLogger = std::shared_ptr<spdlog::logger>;

static std::vector<unsigned short> convert(std::vector<int> channels)
{
    if (channels.size() > 4)
        throw std::runtime_error("Too Many Channels. Expected 4.");

    if (channels.size() < 1)
        throw std::runtime_error("Too Few Channels. Expected At Least 1.");

    std::vector<unsigned short> convertedChannels;

    for (const auto& elem : channels)
    {
        if (elem < 0 || elem > 0xFFFF)
            throw std::runtime_error("Invalid Channel Number");

        convertedChannels.push_back(std::make_unsigned<short>::type(elem));
    }

    return convertedChannels;
}

static PowerSupplyProperties get_crate_map(SpdlogLogger logger, int handle)
{
    PowerSupplyProperties properties;

    // We need the following variables/pointers in order to interface.
    unsigned short numberOfSlots;
    unsigned short* listOfChannelsIndexedBySlot = nullptr;
    char* listOfModelsIndexedBySlot = nullptr;
    char* descriptionList = nullptr;
    unsigned short* listOfSerialNumbersIndexedBySlot = nullptr;
    unsigned char* listOfFirmwareSuffixesIndexedBySlot = nullptr;
    unsigned char* listOfFirmwarePrefixesIndexedBySlot = nullptr;

    auto result = (int) GetCrateMap(
        handle,
        &numberOfSlots,
        &listOfChannelsIndexedBySlot,
        &listOfModelsIndexedBySlot,
        &descriptionList,
        &listOfSerialNumbersIndexedBySlot,
        &listOfFirmwareSuffixesIndexedBySlot,
        &listOfFirmwarePrefixesIndexedBySlot
    );

    if (result)
    {
        std::string msg = "GetCrateMap Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        if (listOfChannelsIndexedBySlot)
            Free(listOfChannelsIndexedBySlot);
        if (listOfModelsIndexedBySlot)
            Free(listOfModelsIndexedBySlot);
        if (descriptionList)
            Free(descriptionList);
        if (listOfSerialNumbersIndexedBySlot)
            Free(listOfSerialNumbersIndexedBySlot);
        if (listOfFirmwareSuffixesIndexedBySlot)
            Free(listOfFirmwareSuffixesIndexedBySlot);
        if (listOfFirmwarePrefixesIndexedBySlot)
            Free(listOfFirmwarePrefixesIndexedBySlot);

        throw std::runtime_error(msg);
    }

    properties.Board = "Board 0";
    properties.Model = listOfModelsIndexedBySlot[0];
    properties.Description = descriptionList[0];
    properties.NumberOfSlots = std::to_string(numberOfSlots);
    properties.ChannelsAvailable = std::to_string(listOfChannelsIndexedBySlot[0]);
    properties.Serial = std::to_string(listOfSerialNumbersIndexedBySlot[0]);
    properties.Firmware = \
        std::to_string(listOfFirmwarePrefixesIndexedBySlot[0]) 
        + "."
        + std::to_string(listOfFirmwareSuffixesIndexedBySlot[0]);

    Free(listOfChannelsIndexedBySlot);
    Free(listOfModelsIndexedBySlot);
    Free(descriptionList);
    Free(listOfSerialNumbersIndexedBySlot);
    Free(listOfFirmwareSuffixesIndexedBySlot);
    Free(listOfFirmwarePrefixesIndexedBySlot);

    return properties;
}

template <typename T>
static void setParameters(std::string parameter, T value, CHVector channels, SpdlogLogger logger, int handle)
{
    std::vector<unsigned short> v;
    try
    {
        v = convert(channels);
    }
    catch(const std::exception& e)
    {
        logger->error("HVInterface Error: {}", e.what());
        throw;
    }

    // We create explicit variables, which will be passed into the function.
    unsigned short slot = 0;
    const char* param = parameter.c_str();

    // BUG: It appears that simultaneous sets to parameters (i.e. passing more
    // than a single channel in) do not actually set all channels. We'll have to
    // iterate then.

    // unsigned short channelListSize = (unsigned short) channels.size();
    // const unsigned short* listOfChannelsToWrite = channels.data();

    unsigned short channelListSize = 1;
    unsigned short listOfChannelsToWrite[1];


    for (int i = 0; i < v.size(); ++i)
    {
        listOfChannelsToWrite[0] = v[i];

        auto result = (int) SetChannelParameter(
            handle,
            slot,
            param,
            channelListSize,
            (const unsigned short*) listOfChannelsToWrite,
            (void*) &value
        );

        if (result)
        {
            std::string msg = "SetChannelParameter Error ";
            msg += std::to_string(result) + ": ";
            msg += GetError(handle);

            if (logger)
                logger->error(msg);

            throw std::runtime_error(msg);
        }

        std::string msg = \
            "CH" + std::to_string(i) + ": "
            + "Parameter " + parameter 
            + " was set to " + std::to_string(value);

        if (logger)
            logger->debug(msg);
    }
}

template <typename T>
static std::vector<T> getParameters(std::string parameter, CHVector channels, SpdlogLogger logger, int handle)
{
    std::vector<unsigned short> v;
    try
    {
        v = convert(channels);
    }
    catch(const std::exception& e)
    {
        logger->error("HVInterface Error: {}", e.what());
        throw;
    }

    unsigned short slot = 0;
    unsigned short channelListSize = (unsigned short) v.size();
    const unsigned short* listOfChannelsToRead = v.data();
    const char* param = parameter.c_str();

    std::vector<T> returnVector(v.size(), (T) 0);

    auto result = (int) GetChannelParameter(
        handle,
        slot,
        param,
        channelListSize,
        listOfChannelsToRead,
        (void*) returnVector.data()
    );

    if (result)
    {
        std::string msg = "GetChannelParameter Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        throw std::runtime_error(msg);
    }

    std::string msg = "Parameter " + parameter + " received: ( ";
    for (int i = 0; i < channelListSize; ++i)
    {
        msg += std::to_string(returnVector[i]) + ", ";
    }
    msg += ")";

    if (logger)
        logger->debug(msg);

    return returnVector;
}

HVInterface::HVInterface():
    handle { -1 },
    connected { false }
{
    try
    {
        logger = spdlog::stdout_color_mt("HVLogger");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        logger = spdlog::get("HVLogger");
    }
}

HVInterface::~HVInterface()
{
    if (!connected)
        return;

    try
    {
        disconnectFromPSU();
    }
    catch (...)
    {
        return;
    }
}

void HVInterface::connectToPSU(msu_smdt::Port port)
{
#ifdef VIRTUALIZE_CONNECTION
    int system = 6;
    int linkType = 5;
#else
    CAENHV_SYSTEM_TYPE_t system = N1470;
    int linkType = LINKTYPE_USB_VCP;
#endif

    auto connection = \
        port.port + "_"
        + port.baud_rate + "_"
        + port.data_bit + "_"
        + port.stop_bit + "_"
        + port.parity + "_"
        + port.lbusaddress;

    const char* username = "";
    const char* password = "";
    int handle = -1;

    auto result = (int) InitializeSystem(
        system,
        linkType,
        (void*) connection.c_str(),
        username,
        password,
        &handle
    );

    if (result)
    {
        std::string msg = "InitializeSystem Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        throw std::runtime_error(msg);
    }

    this->handle = handle;
    this->connected = true;

    if (logger)
        logger->debug("Successfully Connected");
}

void HVInterface::disconnectFromPSU()
{
    auto result = (int) DeinitializeSystem(handle);

    if (result)
    {
        std::string msg = "DeinitializeSystem Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        throw std::runtime_error(msg);
    }

    this->handle = -1;
    this->connected = false;

    if (logger)
        logger->debug("Successfully Disconnected");
}

bool HVInterface::isConnectedToPSU()
{
    return connected;
}

PowerSupplyProperties HVInterface::getProperties()
{
    return properties;
}

void HVInterface::setParametersFloat(std::string parameter, float value, CHVector channels)
{
    try
    {
        setParameters<float>(parameter, value, channels, this->logger, this->handle);
    }
    catch (const std::runtime_error& e)
    {
        logger->error("{}", e.what());
        throw;
    }
    
}

void HVInterface::setParametersLong(std::string parameter, unsigned long value, CHVector channels)
{
    try
    {
        setParameters<unsigned long>(parameter, value, channels, this->logger, this->handle);
    }
    catch (const std::runtime_error& e)
    {
        logger->error("{}", e.what());
        throw;
    }
}

FloatVector HVInterface::getParametersFloat(std::string parameter, CHVector channels)
{
    try
    {
        return getParameters<float>(parameter, channels, this->logger, this->handle);
    }
    catch (const std::runtime_error& e)
    {
        logger->error("{}", e.what());
        throw;
    }
}

ULongVector HVInterface::getParametersLong(std::string parameter, CHVector channels)
{
    try
    {
        return getParameters<unsigned long>(parameter, channels, this->logger, this->handle);
    }
    catch (const std::runtime_error& e)
    {
        logger->error("{}", e.what());
        throw;
    }
}

bool HVInterface::checkAlarm()
{
#ifdef VIRTUALIZE_CONNECTION
    return false;
#else
    unsigned short numberOfSlots = 1;
    const unsigned short slotList[] = { 0 };
    const char* parameter = "Alarm";

    unsigned long resultList[] = { 512 };

    auto result = (int) CAENHV_GetBdParam(
        handle,
        numberOfSlots,
        slotList,
        parameter,
        (void*) resultList
    );

    if (result)
    {
        std::string msg = "CAENHV_GetBdParam Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        throw std::runtime_error(msg);
    }

    if (logger)
        logger->debug("Alarm status: {}", resultList[0]);

    return bool(resultList[0]);
#endif
}

bool HVInterface::checkInterlock()
{
#ifdef VIRTUALIZE_CONNECTION
    return false;
#else
    unsigned short numberOfSlots = 1;
    const unsigned short slotList[] = { 0 };
    const char* parameter = "IlkStat";

    unsigned long resultList[] = { 512 };

    auto result = (int) CAENHV_GetBdParam(
        handle,
        numberOfSlots,
        slotList,
        parameter,
        (void*) resultList
    );

    if (result)
    {
        std::string msg = "CAENHV_GetBdParam Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        throw std::runtime_error(msg);
    }

    if (logger)
        logger->debug("Interlock status: {}", resultList[0]);

    return bool(resultList[0]);
#endif
}

void HVInterface::clearAlarm()
{
#ifdef VIRTUALIZE_CONNECTION
    return;
#else
    unsigned short numberOfSlots = 1;
    const unsigned short slotList[] = { 0 };
    const char* parameter = "ClrAlarm";
    unsigned long value = 0;

    auto result = (int) CAENHV_SetBdParam(
        handle,
        numberOfSlots,
        slotList,
        parameter,
        (void*) &value
    );

    if (result)
    {
        std::string msg = "CAENHV_SetBdParam Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        throw std::runtime_error(msg);
    }
#endif
}

void HVInterface::setInterlock(bool state)
{
#ifdef VIRTUALIZE_CONNECTION
    return;
#else
    unsigned short numberOfSlots = 1;
    const unsigned short slotList[] = { 0 };
    const char* parameter = "Interlock";
    unsigned long value = 0;

    if (state)
        value = 1;

    auto result = (int) CAENHV_SetBdParam(
        handle,
        numberOfSlots,
        slotList,
        parameter,
        (void*) &value
    );

    if (result)
    {
        std::string msg = "CAENHV_SetBdParam Error ";
        msg += std::to_string(result) + ": ";
        msg += GetError(handle);

        if (logger)
            logger->error(msg);

        throw std::runtime_error(msg);
    }
#endif
}
