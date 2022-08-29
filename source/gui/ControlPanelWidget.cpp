#include <QHBoxLayout>

#include "ControlPanelWidget.hpp"

ControlPanelWidget::ControlPanelWidget(QWidget* parent):
    QWidget(parent),
    connected { false },
    started { false },
    connection { new QPushButton("Connect") },
    execution { new QPushButton("Start/Stop") }
{
    execution->setEnabled(false);

    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(connection);
    layout->addWidget(execution);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    connect(
        connection,
        &QPushButton::clicked,
        [this]() {
            if (connected)
                emit userRequestsToDisconnect();
            else
                emit userRequestsToConnect();
        }
    );

    connect(
        execution,
        &QPushButton::clicked,
        [this]() {
            if (connected && !started)
                emit userRequestsToStart();
            else if (connected && started)
                emit userRequestsToStop();
            else
                emit invalidUserRequest();
        }
    );
}

void ControlPanelWidget::receiveStopCommandFromTest()
{
    execution->setText("Start");
}

void ControlPanelWidget::setConnectionState(bool state)
{
    if (state)
    {
        connected = true;
        connection->setText("Disconnect");
        execution->setEnabled(true);
        execution->setText("Start");
    }
    else
    {
        connected = false;
        connection->setText("Connect");
        execution->setEnabled(false);
        execution->setText("Start/Stop");
    }
}

void ControlPanelWidget::setExecutionState(bool state)
{
    if (state)
    {
        started = true;
        execution->setText("Stop");
    }
    else
    {
        started = false;
        execution->setText("Start");
    }
}