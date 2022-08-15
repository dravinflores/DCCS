#pragma once

#include <QWidget>
#include <QPushButton>

#include "TestInfo.hpp"

class ControlPanelWidget : public QWidget
{
    Q_OBJECT

public:
    ControlPanelWidget(QWidget* parent = nullptr);

public slots:
    void connectionChanged(bool status);
    void executionChanged(bool status);

signals:
    void requestToConnect();
    void requestToStart();

private:
    QPushButton* connection;
    QPushButton* execution;
};