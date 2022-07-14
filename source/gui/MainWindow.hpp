#pragma once

#include <string>
#include <vector>

#include <QWidget>
#include <QMainWindow>

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

    void readInTestSettings(std::string path);

public slots:
    void receiveRequestToConnect();
    void receiveRequestToDisconnect();
    void receiveRequestToStart();
    void receiveRequestToStop();

    void alertUser();

signals:

private:
    // TestController* controller;

    QWidget* channelWidgetContainer;

    ControlPanelWidget* controlPanelWidget;
    ChannelWidget* channelWidgetLeft;
    ChannelWidget* channelWidgetRight;
    TestStatusWidget* testStatusWidget;
};