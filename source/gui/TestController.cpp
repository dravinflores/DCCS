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

constexpr int timeout = 30000;

TestController::TestController(QObject* parent):
    connection { false },
    // serial { new DCCHController },
    testThread { new QThread },
    mutex { std::make_shared<QMutex>() },
    controller { std::make_shared<PSUController>() },
    logger { spdlog::stdout_color_mt("TestController") }
{}

TestController::~TestController()
{}

bool TestController::checkConnection() const
{
    return connection;
}

void TestController::setTestParameters(TestParameters parameters)
{
    this->parameters = parameters;
}

void TestController::initializeTestConfiguration(TestConfiguration normal, TestConfiguration reverse)
{
    this->normal = normal;
    this->reverse = reverse;

    std::vector<int> normalChannels;
    std::vector<int> reverseChannels;

    std::vector<unsigned long> polarities;

    try
    {
        polarities = controller->readPolarities({ 0, 1, 2, 3 });
    }
    catch (const std::exception& ex)
    {
        logger->error("{}", ex.what());
        emit alert("Cannot obtain initial polarities. Cannot initialize system");
        return;
    }


    for (int i = 0; i < 4; ++i)
    {
        if (polarities[i])
            reverseChannels.push_back(i);
        else
            normalChannels.push_back(i);
    }

    try
    {
        controller->setTestVoltages(normalChannels, normal.testVoltage);
        controller->setMaxVoltages(normalChannels, normal.maxVoltage);
        controller->setRampUpRate(normalChannels, normal.rampUpRate);
        controller->setRampDownRate(normalChannels, normal.rampDownRate);
        controller->setOverCurrentLimits(normalChannels, normal.overCurrentLimit);

        if (normal.powerDownMethod)
            controller->killChannelsAfterTest(normalChannels, true);
        else
            controller->killChannelsAfterTest(normalChannels, false);

        controller->setTestVoltages(reverseChannels, reverse.testVoltage);
        controller->setMaxVoltages(reverseChannels, reverse.maxVoltage);
        controller->setRampUpRate(reverseChannels, reverse.rampUpRate);
        controller->setRampDownRate(reverseChannels, reverse.rampDownRate);
        controller->setOverCurrentLimits(reverseChannels, reverse.overCurrentLimit);

        if (reverse.powerDownMethod)
            controller->killChannelsAfterTest(reverseChannels, true);
        else
            controller->killChannelsAfterTest(reverseChannels, true);
    }
    catch (const std::exception& ex)
    {
        logger->error("{}", ex.what());
        emit alert("Cannot initialize PSU Channels for testing");
    }
}

bool TestController::connect(msu_smdt::Port PSUPort, msu_smdt::Port DCCHPort)
{
    try
    {
        logger->debug("Connecting to PSU");
        controller->connectToPSU(PSUPort);
        this->DCCHPort = DCCHPort;
        // serial->setPort(DCCHPort);
    }
    catch (const std::exception& ex)
    {
        logger->error("{}", ex.what());
        connection = false;
        return false;
    }

    connection = true;
    return true;
}

bool TestController::disconnect()
{
    try
    {
        controller->disconnectFromPSU();
    }
    catch (const std::exception& ex)
    {
        logger->error("{}", ex.what());
        connection = true;
        return false;
    }

    connection = false;
    return true;
}

void TestController::createNewTest()
{
    if (testThread->isRunning())
    {
        emit finished();
        testThread->quit();
        testThread->wait();
    }
    // testThread->deleteLater();
    // delete testThread;

    // logger->debug("Creating a new test");

    // this->testThread = new QThread;
    Test* test = new Test;

    test->moveToThread(testThread);

    QObject::connect(
        testThread,
        &QThread::finished,
        test,
        &QObject::deleteLater
    );

    // QObject::connect(this, &TestController::executeTestInThread, test, &Test::test);

    QObject::connect(
        test,
        &Test::distributeChannelStatus,
        [this](int channel, std::string status) {
            emit distributeChannelStatus(channel, status);
        }
    );

    QObject::connect(
        test,
        &Test::distributeChannelPolarity,
        [this](int channel, int polarity) {
            emit distributeChannelPolarity(channel, polarity);
        }
    );

    QObject::connect(
        test,
        &Test::distributeTubeDataPacket,
        [this](TubeData data) {
            emit distributeTubeDataPacket(data);
        }
    );

    QObject::connect(
        test,
        &Test::distributeTimeInfo,
        [this](std::string remaining) {
            emit distributeTimeInfo(remaining);
        }
    );

    QObject::connect(
        this,
        &TestController::stopTest,
        test,
        &Test::stop,
        Qt::DirectConnection
    );

    QObject::connect(
        this,
        &TestController::executeTestInThread,
        test,
        &Test::test
    );

    // QObject::connect(test, &Test::connectTube, serial, &DCCHController::connectTube);
    // QObject::connect(test, &Test::disconnectTube, serial, &DCCHController::disconnectTube);

    QObject::connect(
        test,
        &Test::finished,
        [this]() {
            emit finished();
        }
    );
}

void TestController::start(std::vector<int> channels, bool mode)
{
    logger->debug("Call to start");

    if (testThread->isRunning())
    {
        // logger->error("A test is currently running. You should wait or stop the test");
        emit stopTest();
    }

    createNewTest();

    TestConfiguration config;

    if (mode)
        config = this->reverse;
    else
        config = this->normal;

    testThread->start();

    emit executeTestInThread(
        mode,
        mutex.get(),
        DCCHPort,
        controller.get(),
        channels,
        parameters,
        config
    );
}

void TestController::stop()
{
    logger->debug("Called function stop");
    emit stopTest();
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

Test::Test(QObject* parent):
    QObject(parent),
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
{
    logger->debug("TEST DESTROYED");
}

void Test::stop()
{
    stopFlag = true;
    logger->debug("STOPPING");
}

void Test::test(
    bool mode,
    QMutex* mutex,
    msu_smdt::Port DCCHPort,
    PSUController* controller,
    std::vector<int> channels,
    TestParameters parameters,
    TestConfiguration config
)
{
    logger->info("Starting Test");

    constexpr int delay = 250;

    if (stopFlag)
        return;

    QMutexLocker controlLocker(mutex);

    QElapsedTimer timer;
    timer.start();

    DCCHController serial(this, DCCHPort);

    int rampTime = config.testVoltage / config.rampUpRate;

    if (rampTime < 1)
        rampTime = 1;

    if (mode)
    {
        reverseTest(channels, controller, parameters, serial);
        emit finished();
        return;
    }

    std::vector<int> physicalTubeNumber(channels.size());

    // We're going to share data storage, in order to save.
    std::vector<TubeData> data(channels.size());
    std::vector<float> currents(channels.size(), -1.00f);
    std::vector<float> voltages(channels.size(), -1.00f);
    std::vector<std::string> statuses(channels.size(), interpretStatus(0xFFFFFFFF));

    int numberOfTubesConnected = parameters.tubesPerChannel * channels.size();

    for (int tube = 0; tube < numberOfTubesConnected; ++tube)
    {
        serial.disconnectTube(tube);
        QThread::msleep(delay);
    }

    if (stopFlag)
        return;

    auto polarities = controller->readPolarities(channels);

    for (int k = 0; k < channels.size(); ++k)
        emit distributeChannelPolarity(channels[k], polarities[k]);

    auto currentOffset = getIntrinsicCurrent(channels, controller, parameters, rampTime);

    if (stopFlag)
        return;

    int s = 0;
    auto elapsedTime = fmt::format("{} s", s);
    auto remainingTime = fmt::format("{} s", s);

    // controller->powerOnChannels(channels);
    // QThread::sleep(rampTime);

    for (int i = 0; i < parameters.tubesPerChannel; ++i)
    {
        if (stopFlag)
            break;

        for (int k = 0; k < channels.size(); ++k)
        {
            if (stopFlag)
                break;

            physicalTubeNumber[k] = (parameters.tubesPerChannel * k) + i;
            serial.connectTube(physicalTubeNumber[k]);
            QThread::msleep(delay);
        }

        for (int t = 0; t < parameters.secondsPerTube; ++t)
        {
            if (stopFlag)
                break;

            // s = static_cast<int>(timer.elapsed());
            // elapsedTime = fmt::format("{} s", s);
            remainingTime = fmt::format("{} s", (parameters.tubesPerChannel - i) * parameters.secondsPerTube);

            collectData(channels, controller, voltages, currents, statuses);

            for (int k = 0; k < channels.size(); ++k)
            {
                if (stopFlag)
                    break;

                data[k].channel = channels[k];
                data[k].index = i;
                data[k].isActive = true;

                data[k].voltage = voltages[k];
                data[k].current = currents[k] + currentOffset[k];

                data[k].intrinsicCurrent = currentOffset[channels[k]];

                emit distributeTubeDataPacket(data[k]);
                emit distributeChannelStatus(data[k].channel, statuses[k]);
                emit distributeTimeInfo(remainingTime);
            }

            QThread::sleep(1);
        }

        for (int k = 0; k < channels.size(); ++k)
        {
            serial.disconnectTube(physicalTubeNumber[k]);
            QThread::msleep(delay);
            data[k].isActive = false;
            emit distributeTubeDataPacket(data[k]);
        }
    }

    for (int tube = 0; tube < numberOfTubesConnected; ++tube)
    {
        serial.disconnectTube(tube);
        QThread::msleep(delay);
    }

    controller->powerOffChannels(channels);
    QThread::sleep(rampTime);

    emit finished();
    logger->info("Test is complete");
}

std::vector<float> Test::getIntrinsicCurrent(
    std::vector<int>& channels,
    PSUController* controller,
    TestParameters& parameters,
    int rampTime
)
{
    logger->info("Performing offset calculation for channels: [ {} ]", fmt::join(channels, ", "));

    std::vector<float> currentOffsets(channels.size(), 0.00f);
    // std::vector<float> testVoltages(channels.size(), 0.00f);

    // testVoltages = controller->getTestVoltages(channels);
    // controller->setTestVoltages(channels, 0.00f);
    controller->powerOnChannels(channels);
    QThread::sleep(rampTime);
    QThread::sleep(parameters.timeForTestingVoltage);
    currentOffsets = controller->readCurrents(channels);
    // controller->powerOffChannels(channels);

    for (auto& i : currentOffsets)
        i *= 1E3;

    logger->info("Offset are: [ {} ]", fmt::join(currentOffsets, ", "));

    // for (int k = 0; k < channels.size(); ++k)
    // controller->setTestVoltages({ k }, testVoltages[k]);

    return currentOffsets;
}

void Test::collectData(
    std::vector<int> ch,
    PSUController* con,
    std::vector<float>& voltages,
    std::vector<float>& currents,
    std::vector<std::string>& statuses
)
{
    // Current, Voltage, Status

    // std::vector<float> voltages(channels.size(), -1.00f);
    // std::vector<float> currents(channels.size(), -1.00f);
    // std::vector<unsigned long> raw_statuses(channels.size(), 0xFFFFFFFF);
    std::vector<unsigned long> raw_statuses;

    try
    {
        currents = con->readCurrents(ch);
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot read currents");
    }

    try
    {
        voltages = con->readVoltages(ch);
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot read voltages");
    }

    try
    {
        raw_statuses = con->readStatuses(ch);
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot read raw statuses");
    }

    for (int i = 0; i < ch.size(); ++i)
        statuses[i] = interpretStatus(raw_statuses[i]);

    for (auto& i : currents)
        i *= 1E3;
}

void Test::reverseTest(
    std::vector<int>& channels,
    PSUController* controller,
    TestParameters& parameters,
    DCCHController& serial
)
{
    logger->debug("REVERSE");

    int delay = 250;

    std::vector<int> physicalTubeNumber(channels.size());

    // We're going to share data storage, in order to save.
    std::vector<TubeData> data(channels.size());
    std::vector<float> currents(channels.size(), -1.00f);
    std::vector<float> voltages(channels.size(), -1.00f);
    std::vector<std::string> statuses(channels.size(), interpretStatus(0xFFFFFFFF));

    for (int i = 0; i < parameters.tubesPerChannel; ++i)
    {
        if (stopFlag)
            break;

        for (int k = 0; k < channels.size(); ++k)
        {
            if (stopFlag)
                break;

            physicalTubeNumber[k] = (parameters.tubesPerChannel * k) + i;
            serial.connectTube(physicalTubeNumber[k]);
            QThread::msleep(delay);
        }
    }

    controller->powerOnChannels(channels);

    while (!stopFlag);

    for (int i = 0; i < parameters.tubesPerChannel; ++i)
    {
        if (stopFlag)
            break;

        for (int k = 0; k < channels.size(); ++k)
        {
            if (stopFlag)
                break;

            physicalTubeNumber[k] = (parameters.tubesPerChannel * k) + i;
            serial.disconnectTube(physicalTubeNumber[k]);
            QThread::msleep(delay);
        }
    }

    controller->powerOffChannels(channels);
}