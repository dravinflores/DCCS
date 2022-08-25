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
#include "DCCHController.hpp"

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

    bool checkConnection() const;

    void setTestParameters(TestParameters parameters);
    void initializeTestConfiguration(TestConfiguration normal, TestConfiguration reverse);

public slots:
    bool connect(msu_smdt::Port PSUPort, msu_smdt::Port DCCHPort);
    bool disconnect();

    void start(std::vector<int> activeChannels, bool mode);
    void stop();

signals:
    void distributeChannelStatus(int channel, std::string status);
    void distributeChannelPolarity(int channel, int polarity);
    void distributeTubeDataPacket(TubeData data);
    void distributeTimeInfo(std::string remaining);

    void stopTest();

    void alert(std::string message);

    void executeTestInThread(
        bool mode,
        QMutex* mutex,
        msu_smdt::Port DCCHPort,
        PSUController* controller,
        std::vector<int> channels,
        TestParameters parameters,
        TestConfiguration config
    );

    void finished();

private:
    void createNewTest();

private:
    bool connection;
    // DCCHController* serial;
    msu_smdt::Port DCCHPort;
    TestConfiguration normal;
    TestConfiguration reverse;
    TestParameters parameters;
    
    QThread* testThread;
    std::shared_ptr<QMutex> mutex;
    std::shared_ptr<PSUController> controller;
    std::shared_ptr<spdlog::logger> logger;
};

class Test : public QObject
{
    Q_OBJECT

public:
    explicit Test(QObject* parent = nullptr);
    ~Test();

public slots:
    void test(
        bool mode,
        QMutex* mutex,
        msu_smdt::Port DCCHPort,
        PSUController* controller,
        std::vector<int> channels,
        TestParameters parameters,
        TestConfiguration config
    );

    void stop();

signals:
    void distributeChannelStatus(int channel, std::string status);
    void distributeChannelPolarity(int channel, int polarity);
    void distributeTubeDataPacket(TubeData data);
    void distributeTimeInfo(std::string remaining);

    void connectTube(int tube);
    void disconnectTube(int tube);

    void finished();

private:
    void reverseTest(
        std::vector<int>& channels,
        PSUController* controller,
        TestParameters& parameters,
        DCCHController& serial
    );

    std::vector<float> getIntrinsicCurrent(
        std::vector<int>& channels,
        PSUController* controller,
        TestParameters& parameters,
        int rampTime
    );

    void collectData(
        std::vector<int> ch,
        PSUController* con,
        std::vector<float>& voltages,
        std::vector<float>& currents,
        std::vector<std::string>& statuses
    );
    // void collectStatus(int channel, std::string& status);

private:
    // QSerialPort* port;
    QMutex loggerMutex;
    std::atomic<bool> stopFlag;
    std::shared_ptr<spdlog::logger> logger;
};