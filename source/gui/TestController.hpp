#pragma once

#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <utility>

#include <QMutex>
#include <QThread>
#include <QObject>

#include <QSerialPort>

#include <spdlog/spdlog.h>

#include <psu/Port.hpp>
#include <psu/PSUController.hpp>

#include "TestInfo.hpp"

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

    bool checkConnection();

    void initializeTestConfiguration(TestConfiguration normal, TestConfiguration reverse);

    void setTestingParameters(TestParameters parameters);

public slots:
    bool connect(msu_smdt::Port PSUPort, msu_smdt::Port DCCHPort);
    bool disconnect();

    void start(std::vector<int> activeChannels);
    void stop();

    void channelPolarityRequest(int channel);

    // Receive signal from test thread, indicating that data is ready.
    void tubeDataPacketIsReadyFromThread(TubeData data);
    void channelStatusIsReadyFromThread(ChannelStatus status);
    void timeInfoIsReadyFromThread(std::string elapsed, std::string remaining);

signals:
    void connected();
    void disconnected();

    void error(std::string message);

    void stopCurrentTest();

    void executeTestInThread(
        QMutex* mutex,
        QSerialPort* port,
        PSUController* controller,
        std::vector<int> channels,
        TestParameters parameters
    );

    void completeChannelPolarityRequest(int polarity);

    void tubeDataPacketIsReady(TubeData data);
    void channelStatusIsReady(ChannelStatus status);
    void timeInfoIsReady(std::string elapsed, std::string remaining);

private:
    void createThread();

private:
    bool connection;
    TestParameters parameters;
    QThread testThread;
    std::shared_ptr<QMutex> mutex;
    std::shared_ptr<QSerialPort> port;
    std::shared_ptr<PSUController> controller;
    std::shared_ptr<spdlog::logger> logger;
};

class Test : public QObject
{
    Q_OBJECT

public:
    explicit Test();
    ~Test();

public slots:
    void test(
        QMutex* mutex,
        QSerialPort* port,
        PSUController* controller,
        std::vector<int> channels,
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