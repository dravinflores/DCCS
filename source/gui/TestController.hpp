// TestController.hpp

#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <utility>

#include <QMutex>
#include <QThread>
#include <QObject>

#include <spdlog/spdlog.h>

#include <psu/Port.hpp>
#include <psu/PSUController.hpp>

#include "TestInfo.hpp"

enum class TestError
{
    RESERVED,
    ConnectionError,
    FilePathError,
    TestIsRunningError,
    ReadError
};

class TestController : public QObject
{
    Q_OBJECT

public:
    explicit TestController(QObject* parent);
    ~TestController();

    TestController(const TestController&) = delete;
    TestController(TestController&&) = delete;

    TestController& operator=(const TestController&) = delete;
    TestController& operator=(TestController&&) = delete;

    bool connect(msu_smdt::Port port);
    bool disconnect();
    bool checkConnection();

    void start(std::vector<int> activeChannels);
    void stop();

    void initializeTestConfiguration(TestConfiguration normalConfig, TestConfiguration reverseConfig);

    ChannelStatus interpretChannelStatus(int channel);

    void setTestingParameters(TestParameters parameters);

public slots:
    // These slots listen for the test thread to signal that data is ready.
    void channelPolarityRequest(int channel);
    void tubeDataPacketReady(TubeData data);
    void channelStatusReady(ChannelStatus status);
    void timeInfoReady(std::string elapsedTime, std::string remainingTime);

signals:
    void error(TestError error, std::string message);

    void stopTest();

    void executeTestInThread(
        QMutex* mutex, 
        PSUController* controller, 
        std::vector<int> activeChannels, 
        TestParameters parameters
    );

    // Once the test thread has signalled that data is ready, the TestController
    // then sends out the data through these signals.
    void completeChannelPolarityRequest(int polarity);
    void distributeTubeDataPacket(TubeData data);
    void distributeChannelStatus(ChannelStatus status);
    void distributeTimeInfo(std::string elapsedTime, std::string remainingTime);

private:
    void createThread();

private:
    bool connection;
    TestParameters parameters;
    QThread testThread;
    std::shared_ptr<QMutex> mutex;
    std::shared_ptr<PSUController> controller;
    std::shared_ptr<spdlog::logger> logger;
};

// This object exists so that we can put a worker function test into a new thread.
class Test : public QObject
{
    Q_OBJECT

public:
    explicit Test();
    ~Test();

public slots:
    void test(
        QMutex* mutex, 
        PSUController* controller, 
        std::vector<int> activeChannels, 
        TestParameters parameters
    );
    void stop();

signals:
    void returnChannelPolarity(int channel, int polarity);
    void distributeTubeDataPacket(TubeData data);
    void distributeChannelStatus(ChannelStatus status);
    void distributeTimeInfo(std::string elapsedTime, std::string remainingTime);

private:
    QMutex loggerMutex;
    std::atomic<bool> stopFlag;
    std::shared_ptr<spdlog::logger> logger;
};