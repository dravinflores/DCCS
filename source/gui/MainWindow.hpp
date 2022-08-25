#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include <spdlog/spdlog.h>

#include <QWidget>
#include <QMainWindow>

#include <psu/Port.hpp>

#include "TestInfo.hpp"
#include "ChannelWidget.hpp"
#include "TestController.hpp"
#include "TestStatusWidget.hpp"
#include "ControlPanelWidget.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool readInTestSettings();

    void raiseAlert(std::string msg);

    void writeCSV();

public slots:
    void alertUser(std::string msg);

    void receiveRequestToConnect();
    void receiveRequestToStart();

signals:
    void connectionStatusChanged(bool status);
    void executionStatusChanged(bool status);

    void start(std::vector<int> channels, bool mode);
    void stop();

private:
    bool hasStarted;

    std::vector<int> normalChannels;
    std::vector<int> reverseChannels;

    TestController* controller;

    // GUI-based members.
    QWidget* channelWidgetContainer;

    ControlPanelWidget* controlPanelWidget;
    ChannelWidget* channelWidgetLeft;
    ChannelWidget* channelWidgetRight;
    TestStatusWidget* testStatusWidget;

    std::shared_ptr<spdlog::logger> logger;

    std::string csv_path;

    // PSU-based members;
    msu_smdt::Port PSUPort;
    msu_smdt::Port HWPort;
    TestParameters parameters;
    TestConfiguration normalConfig;
    TestConfiguration reverseConfig;
};