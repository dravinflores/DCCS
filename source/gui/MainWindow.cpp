// MainWindow.cpp

#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QMenuBar>
#include <QFormLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

#include <spdlog/sinks/stdout_color_sinks.h>

#include <psu/Port.hpp>
#include <psu/PSUController.hpp>

#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    parameters {
        .secondsPerTube = 4,
        .tubesPerChannel = 16,
        .timeForTestingVoltage = 1,
    },
    controller { new TestController(this) },
    CH0Model { new CollectionModel(this, 0, parameters) },
    CH1Model { new CollectionModel(this, 1, parameters) },
    CH0View { new QTableView },
    CH1View { new QTableView },
    CH0Status { new QLabel("UNKNOWN") },
    CH1Status { new QLabel("UNKNOWN") },
    connectionStatus { new QLabel("Disconnected") },
    timeStarted { new QLabel("00:00") },
    timeElapsed { new QLabel("0 min") },
    timeRemaining { new QLabel("0 min") },
    channelFrame { new QFrame },
    testStatusFrame { new QFrame },
    CH0DataBox { new QGroupBox },
    CH1DataBox { new QGroupBox },
    CH0StatusBox { new QGroupBox },
    CH1StatusBox { new QGroupBox },
    testStatusBox { new QGroupBox },
    connectMenu { new QMenu() },
    testMenu { new QMenu() },
    helpMenu { new QMenu() },
    connectAct { new QAction("Connect to Power Supply", this) },
    disconnectAct { new QAction("Disconnect from Power Supply", this) },
    startAct { new QAction("Start Test", this) },
    stopAct { new QAction("Stop Test", this) },
    openHelpAct { new QAction("Open Help", this) },
    openGitHubAct { new QAction("Open GitHub", this) },
    aboutAct { new QAction("About", this) },
    logger { spdlog::stdout_color_mt("MainWindow") }
{
    // Will most likely remove this.
    // resize(1920, 1080);
    
    // We need to create the central layout.
    QWidget* centralWidget = new QWidget(this);

    // We're going to start from the bottom-up in construction. 
    // This will allow for a much more structured code approach.

    // We'll start with the channel box.

    // Starting with CH0, we'll connect the model to the view first. 
    CH0View->setModel(CH0Model);
    CH0View->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    CH0View->setGridStyle(Qt::NoPen);
    CH0View->setAlternatingRowColors(true);

    QVBoxLayout* CH0DataBoxLayout = new QVBoxLayout;
    CH0DataBoxLayout->addWidget(CH0View);
    CH0DataBox->setLayout(CH0DataBoxLayout);
    CH0DataBox->setTitle("CH0 - Data");

    QVBoxLayout* CH0StatusBoxLayout = new QVBoxLayout;
    QLabel* CH0StatusLabel = new QLabel(CH0Status);
    CH0StatusBox->setLayout(CH0StatusBoxLayout);
    CH0StatusBox->setTitle("CH0 - Status");

    // Onto CH1.
    CH1View->setModel(CH1Model);
    CH1View->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    CH1View->setGridStyle(Qt::NoPen);
    CH1View->setAlternatingRowColors(true);

    QVBoxLayout* CH1DataBoxLayout = new QVBoxLayout;
    CH1DataBoxLayout->addWidget(CH1View);
    CH1DataBox->setLayout(CH1DataBoxLayout);
    CH1DataBox->setTitle("CH1 - Data");

    QVBoxLayout* CH1StatusBoxLayout = new QVBoxLayout;
    QLabel* CH1StatusLabel = new QLabel(CH1Status);
    CH1StatusBox->setLayout(CH1StatusBoxLayout);
    CH1StatusBox->setTitle("CH1 - Status");

    // Now we'll create the frame that holds the data boxes.
    QFrame* dataFrame = new QFrame;
    QGridLayout* dataFrameLayout = new QGridLayout;

    // We'll now add the data widgets.
    dataFrameLayout->addWidget(CH0DataBox, 0, 0);
    dataFrameLayout->addWidget(CH1DataBox, 0, 1);
    dataFrameLayout->addWidget(CH0StatusBox, 1, 0);
    dataFrameLayout->addWidget(CH1StatusBox, 1, 1);
    dataFrame->setLayout(dataFrameLayout);

    // We can now construct the Test Status Widget.
    QFormLayout* testStatusLayout = new QFormLayout;
    QLabel* connectionStatusLabel = new QLabel("Connection Status: ");
    QLabel* timeStartedLabel = new QLabel("Time Started: ");
    QLabel* timeElapsedLabel = new QLabel("Time Elapsed: ");
    QLabel* timeRemainingLabel = new QLabel("Time Remaining: ");
    testStatusLayout->addRow(connectionStatusLabel, connectionStatus);
    testStatusLayout->addRow(timeStartedLabel, timeStarted);
    testStatusLayout->addRow(timeElapsedLabel, timeElapsed);
    testStatusLayout->addRow(timeRemainingLabel, timeRemaining);
    testStatusBox->setLayout(testStatusLayout);
    testStatusBox->setTitle("Test Status");

    // Now we'll create the Test Status Frame.
    QFrame* testFrame = new QFrame;
    QBoxLayout* testFrameLayout = new QHBoxLayout;
    testFrameLayout->addWidget(testStatusBox, Qt::AlignCenter);
    testFrame->setLayout(testFrameLayout);

    // We can now connect the two frames together.
    QVBoxLayout* centralLayout = new QVBoxLayout;
    centralLayout->addWidget(dataFrame);
    centralLayout->addWidget(testFrame);

    centralWidget->setLayout(centralLayout);
    setCentralWidget(centralWidget);

    // We can now create the menus.
    connectMenu = this->menuBar()->addMenu("Connect");
    connectMenu->addAction(connectAct);
    connectMenu->addAction(disconnectAct);

    testMenu = this->menuBar()->addMenu("Test");
    testMenu->addAction(startAct);
    testMenu->addAction(stopAct);

    helpMenu = this->menuBar()->addMenu("Help");
    helpMenu->addAction(openHelpAct);
    helpMenu->addAction(openGitHubAct);
    helpMenu->addAction(aboutAct);

    // Now we're ready to connect all the signals.
    QObject::connect(connectAct, &QAction::triggered, this, &MainWindow::getPortInfoPSU);
    QObject::connect(this, &MainWindow::connect, controller, &TestController::connect);
    QObject::connect(disconnectAct, &QAction::triggered, controller, &TestController::disconnect);
    QObject::connect(startAct, &QAction::triggered, this, &MainWindow::start);
    QObject::connect(stopAct, &QAction::triggered, controller, &TestController::stop);

    QObject::connect(
        controller, 
        &TestController::completeChannelPolarityRequest, 
        CH0Model, 
        &CollectionModel::receiveChannelPolarity
    );

    QObject::connect(
        controller, 
        &TestController::completeChannelPolarityRequest, 
        CH1Model, 
        &CollectionModel::receiveChannelPolarity
    );

    QObject::connect(
        controller, 
        &TestController::distributeTubeDataPacket,
        CH0Model, 
        &CollectionModel::receiveTubeDataPacket
    );

    QObject::connect(
        controller, 
        &TestController::distributeTubeDataPacket,
        CH1Model, 
        &CollectionModel::receiveTubeDataPacket
    );

    QObject::connect(this, &MainWindow::startTest, controller, &TestController::start);
}

MainWindow::~MainWindow()
{}

void MainWindow::start()
{
    logger->debug("Getting to emit startTest");
    emit startTest({0, 1});
}

void MainWindow::getPortInfoPSU()
{
    PSUPort =  {
        "COM4",
        "9600",
        "8",
        "0",
        "0",
        "0"
    };

    emit connect(PSUPort);
}

void MainWindow::openHelp()
{}

void MainWindow::openGitHub()
{}

void MainWindow::about()
{}

void MainWindow::update()
{
    
}