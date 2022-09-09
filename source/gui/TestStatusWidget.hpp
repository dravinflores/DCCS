#pragma once

#include <QTimer>
#include <QLabel>
#include <QWidget>
#include <QGroupBox>

class TestStatusWidget : public QWidget
{
    Q_OBJECT

public:
    TestStatusWidget(QWidget* parent = nullptr);
    void updateConnectionStatus(bool status);
    void startTime();
    void stopTime();
    void receiveTimeRemaining(std::string time);

public slots:
    void update();

private:
    QTimer* timer;

    QLabel* connectionStatus;
    QLabel* timeStarted;
    QLabel* timeElapsed;
    QLabel* timeRemaining;

    QGroupBox* box;
};