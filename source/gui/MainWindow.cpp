#include "MainWindow.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    // controller { new TestController(this) },
    channelWidgetContainer { new QWidget },
    controlPanelWidget { new ControlPanelWidget },
    channelWidgetLeft { new ChannelWidget },
    channelWidgetRight { new ChannelWidget },
    testStatusWidget { new TestStatusWidget }
{
    QVBoxLayout* layout = new QVBoxLayout;
    QHBoxLayout* channelWidgetLayout = new QHBoxLayout;

    channelWidgetLayout->addWidget(channelWidgetLeft);
    channelWidgetLayout->addWidget(channelWidgetRight);
    channelWidgetContainer->setLayout(channelWidgetLayout);

    layout->addWidget(controlPanelWidget);
    layout->addWidget(channelWidgetContainer);
    layout->addWidget(testStatusWidget);

    QWidget* centralWidget = new QWidget;
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Testing Purposes
    channelWidgetLeft->setChannel(0);
    channelWidgetRight->setChannel(1);

    channelWidgetLeft->receiveChannelPolarity(-1);
    channelWidgetRight->receiveChannelPolarity(+1);
}

MainWindow::~MainWindow()
{}

void MainWindow::readInTestSettings(std::string path)
{}

void MainWindow::receiveRequestToConnect()
{}

void MainWindow::receiveRequestToDisconnect()
{}

void MainWindow::receiveRequestToStart()
{}

void MainWindow::receiveRequestToStop()
{}

void MainWindow::alertUser()
{}
