// TestController.cpp

#include <cstdlib> // for rand()

#include <map>

#include <QMutexLocker>
#include <QElapsedTimer>

#include <fmt/core.h>
#include <fmt/ranges.h>

#include <spdlog/sinks/stdout_color_sinks.h>

#include "TestController.hpp"

TestController::TestController(QObject* parent):
    QObject(parent),
    connection { false },
    parameters { 10, 8, 1 },
    mutex { std::make_shared<QMutex>() },
    controller { std::make_shared<PSUController>() },
    logger { spdlog::stdout_color_mt("TestController") }
{}

TestController::~TestController()
{}

bool TestController::connect(msu_smdt::Port port)
{
    try
    {
        logger->debug("Connecting");
        controller->connectToPSU(port);
    }
    catch(const std::exception& e)
    {
        logger->error("{}", e.what());
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
    catch(const std::exception& e)
    {
        logger->error("{}", e.what());
        connection = true;
        return false;
    }

    connection = false;
    return true;
}

bool TestController::checkConnection()
{
    return connection;
}

void TestController::initializeTestConfiguration(TestConfiguration normalConfig, TestConfiguration reverseConfig)
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
        controller->setTestVoltages(normalChannels, normalConfig.testVoltage);
        // controller->setCurrentLimits(normalChannels, normalConfig.currentLimit);
        controller->setMaxVoltages(normalChannels, normalConfig.maxVoltage);
        controller->setRampUpRate(normalChannels, normalConfig.rampUpRate);
        controller->setRampDownRate(normalChannels, normalConfig.rampDownRate);
        controller->setOverCurrentLimits(normalChannels, normalConfig.overCurrentLimit);

        if (normalConfig.powerDownMethod)
            controller->killChannelsAfterTest(normalChannels, true);
        else
            controller->killChannelsAfterTest(normalChannels, true);

        controller->setTestVoltages(reverseChannels, reverseConfig.testVoltage);
        // controller->setCurrentLimits(reverseChannels, reverseConfig.currentLimit);
        controller->setMaxVoltages(reverseChannels, reverseConfig.maxVoltage);
        controller->setRampUpRate(reverseChannels, reverseConfig.rampUpRate);
        controller->setRampDownRate(reverseChannels, reverseConfig.rampDownRate);
        controller->setOverCurrentLimits(reverseChannels, reverseConfig.overCurrentLimit);

        if (reverseConfig.powerDownMethod)
            controller->killChannelsAfterTest(reverseChannels, true);
        else
            controller->killChannelsAfterTest(reverseChannels, true);
    }
    catch (const std::exception& e)
    {
        logger->error("{}", e.what());
    }
}

ChannelStatus TestController::interpretChannelStatus(int channel)
{
    // Not Yet Implemented.
    ChannelStatus status;
    return status;
}

void TestController::setTestingParameters(TestParameters parameters)
{
    this->parameters = parameters;
}

void TestController::start(std::vector<int> activeChannels)
{
    createThread();

    logger->debug("Checking if a test is running");

    if (testThread.isRunning())
    {
        logger->error("A Test is currently running. You should wait or stop the test");
        // emit this->error(TestError::TestIsRunningError, "A Test is currently running. You should wait or stop the test");
    }
    else
    {
        // We need a way of clearing a test.

        testThread.start();
        emit executeTestInThread(
            mutex.get(), 
            controller.get(), 
            activeChannels, 
            parameters
        );
    }
}

void TestController::stop()
{
    emit stopTest();
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
        emit this->error(TestError::ReadError, e.what());
    }

    emit completeChannelPolarityRequest(polarity);
}

void TestController::tubeDataPacketReady(TubeData data)
{
    emit distributeTubeDataPacket(data);
}

void TestController::channelStatusReady(ChannelStatus status)
{
    emit distributeChannelStatus(status);
}

void TestController::timeInfoReady(std::string elapsedTime, std::string remainingTime)
{
    emit distributeTimeInfo(elapsedTime, remainingTime);
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
    QObject::connect(test, &Test::distributeTubeDataPacket, this, &TestController::tubeDataPacketReady);
    QObject::connect(test, &Test::distributeChannelStatus, this, &TestController::channelStatusReady);
    QObject::connect(test, &Test::distributeTimeInfo, this, &TestController::timeInfoReady);

    // We want to connect the stop signals.
    QObject::connect(this, &TestController::stopTest, test, &Test::stop, Qt::DirectConnection);
}

static void packageTubeData(std::shared_ptr<spdlog::logger> logger, PSUController* controller, TubeData& data)
{
    try
    {
        data.current = controller->readCurrents({ data.channel })[0];
    }
    catch (...)
    {
        logger->error("Error. Cannot obtain current");
    }

    try
    {
        data.voltage = controller->readVoltages({ data.channel })[0];
    }
    catch (...)
    {
        logger->error("Error. Cannot obtain voltage");
    }

    data.current += (float) (rand() % 60) + 0.01;
    data.voltage += (float) (rand() % 60) + 0.06;
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

void Test::test(QMutex* mutex, PSUController* controller, std::vector<int> activeChannels, TestParameters parameters)
{
    // We haven't received a stop call yet seeing as we just started. This
    // variable is atomic, so it is safe to read and write without guarding
    // with a mutex. 
    stopFlag = false;

    // We need to lock the mutex so we can access TestController exclusively.
    QMutexLocker controlLocker(mutex);
    // QMutexLocker log(loggerMutex);

    // We're now in a different thread. Let's get to work.

    // We'll start a timer to record the elapsed time, remaining time, etc.
    QElapsedTimer timer;
    timer.start();

    logger->debug("Beginning Testing");

    // We can, in principle, perform a test on mutiple channels simultaneously.
    // Critical Assumption: The activeChannels vector is sorted.

    // If we consider that the Critical Assumption is true, then we can make the
    // following optimization. We allow for each channel to have, at most, a
    // single tube testing at a time. Since there are only ever 4 active
    // channels, then we only need to store the data for 4 tubes. We can just
    // recycle this data for each iteration, effectively eliminating the need
    // to allocate memory for every tube.
    int size = activeChannels.size();
    std::vector<TubeData> data(size);

#define OFFSET
#ifdef OFFSET
    // We can now find out the currentOffset.
    std::vector<float> currentOffsets(size, 0.00f);
    std::vector<float> testVoltages(size, 0.00f);

    logger->info("Performing offset calculation for channels: [ {} ]", fmt::join(activeChannels, ", "));

    if (!stopFlag)
    {
        testVoltages = controller->getTestVoltages(activeChannels);
        controller->setTestVoltages(activeChannels, 0.00f);
        controller->powerOnChannels(activeChannels);
        QThread::sleep(parameters.timeForTestingVoltage);
        currentOffsets = controller->readCurrents(activeChannels);

        logger->info("Offsets are: [ {} ]", fmt::join(currentOffsets, ", "));

        for (int k = 0; k < size; ++k)
            controller->setTestVoltages({ k }, testVoltages[k]);
    }
#else
    // We'll power on each channel and ramp to the testing voltage.
    for (int k = 0; k < size; ++k)
    {
        if (stopFlag)
            break;

        controller->powerChannelOn(k);
    }
#endif

    int minutes = 0;

    // Each tube has a uniform number of tubes in a channel. 
    // Each tube is indexed in the channel.
    for (int i = 0; i < parameters.tubesPerChannel; ++i)
    {
        if (stopFlag)
            break;

        // We have no way of knowing whether we're testing channels { 0, 2 } or
        // channels { 0, 1, 2, 3 } ahead of time. So we'll use an index to
        // gather channels, meaning we're at the kth channel.
        for (int k = 0; k < size; ++k)
        {
            if (stopFlag)
                break;

            // Physically connect the tube.
            // hwcontroller.connect(activeChannels[k], i);

            data[k].channel = activeChannels[k];
            data[k].index = i;
            data[k].isActive = true;
        }

        // Now let's test each tube.
        for (int t = 0; t < parameters.secondsPerTube; ++t)
        {
            if (stopFlag)
                break;

            for (int k = 0; k < size; ++k)
            {
                logger->debug("Collecting for channel {}", k);

                int s = timer.elapsed() / 1000;
                auto elapsedTime = fmt::format("{} s", s);
                
                auto remainingTime = fmt::format("{} s", (parameters.tubesPerChannel - i) * parameters.secondsPerTube);
                ChannelStatus status;
                
                logger->debug("Collecting for channel {}", k);
                packageTubeData(logger, controller, data[k]);
                emit distributeTubeDataPacket(data[k]);
                // emit distributeChannelStatus(status);
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

    logger->debug("Testing Complete");
}

void Test::stop()
{
    stopFlag = true;
}