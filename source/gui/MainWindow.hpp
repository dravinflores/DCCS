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

    void readInTestSettings();

public slots:
    void receiveRequestToConnect();
    void receiveRequestToDisconnect();
    void receiveRequestToStart();
    void receiveRequestToStop();

    void alertUser();

signals:

private:
    // TestController* controller;

    // GUI-based members.
    QWidget* channelWidgetContainer;

    ControlPanelWidget* controlPanelWidget;
    ChannelWidget* channelWidgetLeft;
    ChannelWidget* channelWidgetRight;
    TestStatusWidget* testStatusWidget;

    std::shared_ptr<spdlog::logger> logger;

    // PSU-based members;
    msu_smdt::Port PSUPort;
    msu_smdt::Port HWPort;
    TestParameters parameters;
    TestConfiguration normalConfig;
    TestConfiguration reverseConfig;
};