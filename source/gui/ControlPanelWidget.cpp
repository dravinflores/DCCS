#include <QHBoxLayout>

#include "ControlPanelWidget.hpp"

ControlPanelWidget::ControlPanelWidget(QWidget* parent):
    QWidget(parent),
    connection { new QPushButton("Connect") },
    execution { new QPushButton("Start/Stop") }
{
    execution->setEnabled(false);

    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(connection);
    layout->addWidget(execution);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    connect(connection, &QPushButton::clicked, this, &ControlPanelWidget::requestToConnect);
    connect(execution, &QPushButton::clicked, this, &ControlPanelWidget::requestToStart);
}

void ControlPanelWidget::connectionChanged(bool status)
{
    if (status)
    {
        connection->setText("Disconnect");
        execution->setText("Start");
        execution->setEnabled(true);
    }
    else
    {
        connection->setText("Connect");
        execution->setText("Start/Stop");
        execution->setEnabled(false);
    }
}

void ControlPanelWidget::executionChanged(bool status)
{
    if (status)
        execution->setText("Stop");
    else
        execution->setText("Start");
}
