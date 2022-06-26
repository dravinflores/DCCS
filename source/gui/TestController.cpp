// TestController.cpp

#include <cstdlib> // for rand()

#include <map>

#include <QMutexLocker>
#include <QElapsedTimer>

#include <fmt/core.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "TestController.hpp"

TestController::TestController(QObject* parent):
    connection { false },
    parameters { 10, 8, 1 },
    mutex { std::make_shared<QMutex>() },
    controller { std::make_shared<PSUController>() },
    logger { spdlog::stdout_color_mt("TestController") }
{}

TestController::~TestController()
{}

void TestController::connect(msu_smdt::Port port)
{
    try
    {
        controller->connect(port);
    }
    catch(const std::exception& e)
    {
        logger->error("{}", e.what());
        connection = false;
        emit this->error(TestError::ConnectionError, "Cannot connect to the power supply.");
    }

    connection = true;
}

void TestController::disconnect()
{
    try
    {
        controller->disconnect();
    }
    catch(const std::exception& e)
    {
        logger->error("{}", e.what());
        connection = true;
        emit this->error(TestError::ConnectionError, "Cannot disconnect to the power supply.");
    }

    connection = false;
}

bool TestController::checkConnection()
{
    return connection;
}

void TestController::initializeTestingParameters()
{
    bool pathIsCorrect = false;

    if (!pathIsCorrect)
    {
        for (int i = 0; i < 4; ++i)
        {
            // Set defaults.
            controller->setTestVoltage(i, 5.00f);
            controller->setCurrentLimit(i, 2.00f);
            controller->setMaxVoltage(i, 10.00f);
            controller->setRampUpRate(i, 1.00f);
            controller->setRampDownRate(i, 1.00f);
            controller->setOverCurrentTimeLimit(i, 1000.00f);
            controller->setPowerDownMethod(i, RecoveryMethod::Ramp);
        }
    }
    else
    {
        emit this->error(TestError::FilePathError, "Invalid file path to configuration file");
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
        polarity = (int) controller->readPolarity(channel);
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
    bool isInHighPrecision = false;

    try
    {
        isInHighPrecision = controller->isInHighPrecision(data.channel);
    }
    catch (...)
    {
        logger->error("Error. Cannot obtain bool isInHighPrecision");
    }

    if (isInHighPrecision)
    {
        try
        {
            data.current = controller->readHighPrecisionCurrent(data.channel);
        }
        catch (...)
        {
            logger->error("Error. Cannot obtain current precision");
        }
    }
    else
    {
        try
        {
            data.current = controller->readLowPrecisionCurrent(data.channel);
        }
        catch (...)
        {
            logger->error("Error. Cannot obtain current");
        }
    }

    try
    {
        data.voltage = controller->readVoltage(data.channel);
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

#ifdef OFFSET
    // We can now find out the currentOffset.
    float currentOffset[size];

    for (int k = 0; k < size; ++k)
    {
        if (stopFlag)
            break

        float testVoltage = controller->getTestVoltage(activeChannels[k]);
        controller->setTestVoltage(activeChannels[k], 0.00f);
        controller->powerChannelOn(k);
        QThread::sleep(10);
        currentOffset[k] = controller->readLowPrecisionCurrent(activeChannels[k]);
        controller->setTestVoltage(activeChannels[k], testVoltage);
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

    // We should read back the test voltage to be sure everything works.
#ifdef COMMENT
    float testVoltage = controller->getTestVoltage(activeChannels[k]);
    
    for (int k = 0; k < size; ++k)
    {
        float epsilon = 0.01;
        float voltage = controller->readVoltage(activeChannels[k]);
        if ((voltage <= testVoltage - epsilon) || (voltage >= testVoltage + epsilon))
        {
            logger->error("Incorrect Voltage");
        }
    }
#endif // COMMENT

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

            // We should gather intrinsic current here.
            // gatherIntrinsicCurrent();

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
                auto elapsedTime = fmt::format("{}", timer.elapsed());
                auto remainingTime = "";
                ChannelStatus status;
                
                logger->debug("Collecting for channel {}", k);
                packageTubeData(logger, controller, data[k]);
                emit distributeTubeDataPacket(data[k]);
                // emit distributeChannelStatus(status);
                // emit distributeTimeInfo(elapsedTime, remainingTime);
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