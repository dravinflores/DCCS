// TestController.cpp

#include <cstdlib> // for rand()

#include <map>

#include <QString>
#include <QByteArray>
#include <QMutexLocker>
#include <QElapsedTimer>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <spdlog/sinks/stdout_color_sinks.h>

#include "TestController.hpp"

constexpr bool USE_HV_WRAPPER_STATUS { true };

constexpr uint32_t ON                   { 1 << 0 };
constexpr uint32_t RAMP_UP              { 1 << 1 };
constexpr uint32_t RAMP_DOWN            { 1 << 2 };
constexpr uint32_t OVER_CURRENT         { 1 << 3 };
constexpr uint32_t OVER_VOLTAGE         { 1 << 4 };
constexpr uint32_t UNDER_VOLTAGE        { 1 << 5 };
constexpr uint32_t EXTERNAL_TRIP        { 1 << 6 };
constexpr uint32_t MAXIMUM_VOLTAGE      { 1 << 7 };
constexpr uint32_t EXTERNAL_DISABLE     { 1 << 8 };
constexpr uint32_t INTERNAL_TRIP        { 1 << 9 };
constexpr uint32_t CALIBRATION_ERROR    { 1 << 10 };
constexpr uint32_t UNPLUGGED            { 1 << 11 };

constexpr int timeout = 5000;

TestController::TestController(QObject* parent):
    QObject(parent),
    connection { false },
    parameters { 10, 8, 1 },
    mutex { std::make_shared<QMutex>() },
    port { std::make_shared<QSerialPort>(this) },
    controller { std::make_shared<PSUController>() },
    logger { spdlog::stdout_color_mt("TestController") }
{}

TestController::~TestController()
{}

bool TestController::checkConnection()
{
    return connection;
}

void TestController::initializeTestConfiguration(TestConfiguration normal, TestConfiguration reverse)
{
    // We need to get the polarities of each channel.
    std::vector<int> normalChannels;
    std::vector<int> reverseChannels;

    try
    {
        for (int i = 0; i < 4; ++i)
        {
            auto polarity = controller->readPolarities({ i });

            if (polarity[0])
                normalChannels.push_back(i);
            else
                reverseChannels.push_back(i);
        }
    }
    catch (const std::exception& e)
    {
        logger->error("{}", e.what());
    }

    try
    {
        controller->setTestVoltages(normalChannels, normal.testVoltage);
        // controller->setCurrentLimits(normalChannels, normal.currentLimit);
        controller->setMaxVoltages(normalChannels, normal.maxVoltage);
        controller->setRampUpRate(normalChannels, normal.rampUpRate);
        controller->setRampDownRate(normalChannels, normal.rampDownRate);
        controller->setOverCurrentLimits(normalChannels, normal.overCurrentLimit);

        if (normal.powerDownMethod)
            controller->killChannelsAfterTest(normalChannels, true);
        else
            controller->killChannelsAfterTest(normalChannels, true);

        controller->setTestVoltages(reverseChannels, reverse.testVoltage);
        // controller->setCurrentLimits(reverseChannels, reverse.currentLimit);
        controller->setMaxVoltages(reverseChannels, reverse.maxVoltage);
        controller->setRampUpRate(reverseChannels, reverse.rampUpRate);
        controller->setRampDownRate(reverseChannels, reverse.rampDownRate);
        controller->setOverCurrentLimits(reverseChannels, reverse.overCurrentLimit);

        if (reverse.powerDownMethod)
            controller->killChannelsAfterTest(reverseChannels, true);
        else
            controller->killChannelsAfterTest(reverseChannels, true);
    }
    catch (const std::exception& e)
    {
        logger->error("{}", e.what());
    }
}

void TestController::setTestingParameters(TestParameters parameters)
{
    this->parameters = parameters;
}

bool TestController::connect(msu_smdt::Port PSUPort, msu_smdt::Port DCCHPort)
{
    try
    {
        logger->debug("Connecting");
        controller->connectToPSU(PSUPort);
    }
    catch(const std::exception& e)
    {
        logger->error("{}", e.what());
        connection = false;
        return false;
    }

    port->setPortName(QString::fromStdString(DCCHPort.port));
    port->setBaudRate(std::stoi(DCCHPort.baud_rate));
    port->setDataBits(QSerialPort::Data8);

    connection = true;
    return true;
}

bool TestController::disconnect()
{
    try
    {
        controller->disconnectFromPSU();
    }
    catch(const std::exception& e)
    {
        logger->error("{}", e.what());
        connection = true;
        return false;
    }

    connection = false;
    return true;
}

void TestController::start(std::vector<int> activeChannels)
{
    createThread();

    logger->debug("Checking if a test is running");

    if (testThread.isRunning())
    {
        logger->error("A Test is currently running. You should wait or stop the test");
    }
    else
    {
        // We need a way of clearing a test.

        testThread.start();
        emit executeTestInThread(
            mutex.get(), 
            port.get(),
            controller.get(), 
            activeChannels, 
            parameters
        );
    }
}

void TestController::stop()
{
    emit stop();
    testThread.quit();
}

void TestController::channelPolarityRequest(int channel)
{
    // We need to lock the mutex before accessing controller.
    QMutexLocker locker(mutex.get());
    int polarity = 0;

    try
    {
        polarity = controller->readPolarities({channel})[0];
    }
    catch (const std::exception& e)
    {
        logger->error("{}", e.what());
        emit error(e.what());
    }

    emit completeChannelPolarityRequest(polarity);
}

void TestController::tubeDataPacketIsReadyFromThread(TubeData data)
{
    emit tubeDataPacketIsReady(data);
}

void TestController::channelStatusIsReadyFromThread(ChannelStatus status)
{
    emit channelStatusIsReady(status);
}

void TestController::timeInfoIsReadyFromThread(std::string elapsed, std::string remaining)
{
    emit timeInfoIsReady(elapsed, remaining);
}

void TestController::createThread()
{
    logger->debug("Creating a new test");
    Test* test = new Test;

    if (!test)
        logger->error("Could not create a new test. Need recovery method");

    test->moveToThread(&testThread);
    QObject::connect(&testThread, &QThread::finished, test, &QObject::deleteLater);
    QObject::connect(this, &TestController::executeTestInThread, test, &Test::test);

    // We now connect the signals from test to the slots in TestController.
    QObject::connect(test, &Test::returnChannelPolarity, this, &TestController::channelPolarityRequest);
    QObject::connect(test, &Test::distributeTubeDataPacket, this, &TestController::tubeDataPacketIsReadyFromThread);
    QObject::connect(test, &Test::distributeChannelStatus, this, &TestController::channelStatusIsReadyFromThread);
    QObject::connect(test, &Test::distributeTimeInfo, this, &TestController::timeInfoIsReadyFromThread);

    // We want to connect the stop signals.
    QObject::connect(this, &TestController::stop, test, &Test::stop, Qt::DirectConnection);
}

static std::string interpretStatus(unsigned long status)
{
    std::string status_str = "";

    if (status & ON)
        status_str += "| ON ";

    if (status & RAMP_UP)
        status_str += "| RAMP_UP ";

    if (status & RAMP_DOWN)
        status_str += "| RAMP_DOWN ";

    if (status & OVER_CURRENT)
        status_str += "| OVER_CURRENT ";

    if (status & OVER_VOLTAGE)
        status_str += "| OVER_VOLTAGE ";

    if (status & UNDER_VOLTAGE)
        status_str += "| UNDER_VOLTAGE ";

    if (status & EXTERNAL_TRIP)
        status_str += "| EXTERNAL_TRIP ";

    if (status & MAXIMUM_VOLTAGE)
        status_str += "| MAX_VOLTAGE ";

    if (status & EXTERNAL_DISABLE)
        status_str += "| EXTERNAL_DISABLE ";

    if (status & INTERNAL_TRIP)
        status_str += "| INTERNAL_TRIP ";

    if (status & CALIBRATION_ERROR)
        status_str += "| CALIBRATION_ERROR ";

    if (status & UNPLUGGED)
        status_str += "| UNPLUGGED ";

    status_str += "|";
    
    return status_str;
}

Test::Test():
    stopFlag { false }
{
    try
    {
        logger = spdlog::stdout_color_mt("Test");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        logger = spdlog::get("Test");
    }
}

Test::~Test()
{}

void Test::test(
    QMutex* mutex, 
    QSerialPort* port,
    PSUController* controller, 
    std::vector<int> channels, 
    TestParameters parameters
)
{
    stopFlag = false;

    QMutexLocker controlLocker(mutex);

    QElapsedTimer timer;
    timer.start();

    if (!port->open(QIODeviceBase::ReadWrite))
    {
        logger->error("Cannot connect to DCCH Board [FATAL]: {}", port->errorString().toStdString());
        return;
    }

    // int rampTime = config.testVoltage / config.rampUpRate;
    int rampTime = 5;

    char buf[] = { '{', '*', '*', '}' };

    logger->info("Starting Test");

    int size = channels.size();
    std::vector<TubeData> data(size);
    std::vector<ChannelStatus> statuses(size);
    
    int numberOfTubesConnected = parameters.tubesPerChannel * size;

    // We are going to disconnect all tubes.
    for (int i = 0; i < numberOfTubesConnected; ++i)
    {
        buf[0] = (char) i;
        buf[1] = (char) 0;
        // auto r = port->writeData(buf, 4);
        // QByteArray arr(buf, 4);
        auto r = port->write(buf, 4);

        if (!port->waitForBytesWritten(timeout))
        {
            logger->error("Cannot send disable command to DCCH [Fatal]");
            return;
        }
    }

    std::vector<float> currentOffsets(size, 0.00f);
    std::vector<float> testVoltages(size, 0.00f);

    logger->info("Performing offset calculation for channels: [ {} ]", fmt::join(channels, ", "));

    if (!stopFlag)
    {
        testVoltages = controller->getTestVoltages(channels);
        controller->setTestVoltages(channels, 0.00f);
        controller->powerOnChannels(channels);
        QThread::sleep(parameters.timeForTestingVoltage);
        currentOffsets = controller->readCurrents(channels);
        controller->powerOffChannels(channels);

        logger->info("Offset are: [ {} ]", fmt::join(currentOffsets, ", "));

        for (int k = 0; k < size; ++k)
            controller->setTestVoltages({ k }, testVoltages[k]);
    }

    int minutes = 0;

    std::vector<int> physicalTubeNumber(size);

    std::vector<float> currents;
    std::vector<float> voltages;
    std::vector<unsigned long> unconverted_statuses;

    for (int i = 0; i < parameters.tubesPerChannel; ++i)
    {
        for (int k = 0; k < size; ++k)
        {
            physicalTubeNumber[k] = (parameters.tubesPerChannel * k) + i;

            buf[1] = (char) physicalTubeNumber[k];
            buf[2] = (char) 0;

            auto r = port->write(buf, 4);
            if (!port->waitForBytesWritten(timeout))
            {
                logger->error("Cannot send enable command to DCCH [Fatal]");
                return;
            }
        }

        for (int t = 0; t < parameters.secondsPerTube; ++t)
        {
            if (stopFlag)
                break;

            int s = timer.elapsed() / 1000;
            auto elapsedTime = fmt::format("{} s", s);
            auto remainingTime = fmt::format("{} s", (parameters.tubesPerChannel - i) * parameters.secondsPerTube);
            
            try
            {
                currents = controller->readCurrents(channels);
            }
            catch (...)
            {
                logger->error("Cannot obtain currents for index {}", i);
            }

            try
            {
                voltages = controller->readVoltages(channels);
            }
            catch (...)
            {
                logger->error("Cannot obtain voltages for index {}", i);
            }

            try
            {
                unconverted_statuses = controller->readStatuses(channels);
            }
            catch (...)
            {
                logger->error("Cannot obtain statuses for index {}", i);
            }

            for (int k = 0; k < size; ++k)
            {
                if (stopFlag)
                    break;

                for (const auto& s : unconverted_statuses)
                {
                    statuses[k].channel = channels[k];
                    statuses[k].status = interpretStatus(s);
                }

                data[k].channel = channels[k];
                data[k].index = physicalTubeNumber[k];
                data[k].isActive = true;

                emit distributeTubeDataPacket(data[k]);
                emit distributeChannelStatus(statuses[k]);
                emit distributeTimeInfo(elapsedTime, remainingTime);
            }

            QThread::sleep(1);
        }

        for (int k = 0; k < size; ++k)
        {
            data[k].isActive = false;
            emit distributeTubeDataPacket(data[k]);
        }
    }

    logger->info("Testing Complete");
}

void Test::stop()
{
    stopFlag = true;
}