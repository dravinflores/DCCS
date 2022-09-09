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

    connection->setCheckable(true);
    execution->setCheckable(true);

    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(connection);
    layout->addWidget(execution);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    connect(
        connection,
        &QPushButton::clicked,
        [this](bool checked) {
            // checked => button pushed in.

            if (checked)
                emit userRequestsToDisconnect();
            else
                emit userRequestsToConnect();
        }
    );

    connect(
        execution,
        &QPushButton::clicked,
        [this](bool checked) {
            if (!connected)
                emit invalidUserRequest();
            else if (checked)
                emit userRequestsToStart();
            else
                emit userRequestsToStop();  
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