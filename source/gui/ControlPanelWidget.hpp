#pragma once

#include <QWidget>
#include <QPushButton>

#include "TestInfo.hpp"

class ControlPanelWidget : public QWidget
{
    Q_OBJECT

public:
    ControlPanelWidget(QWidget* parent = nullptr);

    void receiveStopCommandFromTest();

    void setConnectionState(bool state);
    void setExecutionState(bool state);

signals:
    void userRequestsToConnect();
    void userRequestsToDisconnect();
    void userRequestsToStart();
    void userRequestsToStop();
    void invalidUserRequest();

private:
    bool connected;
    bool started;

    QPushButton* connection;
    QPushButton* execution;
};