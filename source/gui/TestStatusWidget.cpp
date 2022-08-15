#include <string>

#include <QTime>
#include <QFormLayout>
#include <QGridLayout>

#include "TestStatusWidget.hpp"

static int seconds;
static int minutes;

TestStatusWidget::TestStatusWidget(QWidget* parent):
    QWidget(parent),
    timer { new QTimer(this) },
    connectionStatus { new QLabel("DISCONNECTED", this) },
    timeStarted { new QLabel("--:--", this) },
    timeElapsed { new QLabel("0 min", this) },
    timeRemaining { new QLabel("0 min", this) },
    box { new QGroupBox("Test Status", this) }
{
    connect(timer, &QTimer::timeout, this, &TestStatusWidget::update);
    seconds = 0;
    minutes = 0;

    QFormLayout* boxLayout = new QFormLayout;
    boxLayout->addRow("Connection Status:", connectionStatus);
    boxLayout->addRow("Time Started:", timeStarted);
    boxLayout->addRow("Time Elapsed:", timeElapsed);
    boxLayout->addRow("Time Remaining:", timeRemaining);
    box->setLayout(boxLayout);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(box);
    layout->setAlignment(Qt::AlignCenter);

    setLayout(layout);
}

void TestStatusWidget::updateConnectionStatus(bool status)
{
    if (status)
        connectionStatus->setText("CONNECTED");
    else
        connectionStatus->setText("DISCONNECTED");
}

void TestStatusWidget::startTime()
{
    timer->start(1000);
    timeStarted->setText(QTime::currentTime().toString("hh:mm:ss"));
}

void TestStatusWidget::stopTime()
{
    timer->stop();
    timeStarted->setText("--:--");
    seconds = 0;
    minutes = 0;

    timeRemaining->setText("0 min");
}

void TestStatusWidget::receiveTimeRemaining(std::string time)
{
    timeRemaining->setText(QString::fromStdString(time));
}

void TestStatusWidget::update()
{
    ++seconds;

    if (seconds == 60)
    {
        ++minutes;
        seconds = 0;
    }

    std::string t;

    if (minutes == 0)
        t = std::to_string(seconds) + " s";
    else
        t = std::to_string(minutes) + ":" + std::to_string(seconds) + " min";

    timeElapsed->setText(QString::fromStdString(t));
}