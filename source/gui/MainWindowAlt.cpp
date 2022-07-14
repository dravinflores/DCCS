// MainWindow.cpp

#include <iostream>
#include <fstream>
#include <utility>

#include <string_view>

#include <QTime>
#include <QLabel>
#include <QFrame>
#include <QWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QFormLayout>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QInputDialog>

#include <spdlog/sinks/stdout_color_sinks.h>

#include <nlohmann/json.hpp>

#include <psu/Port.hpp>
#include <psu/PSUController.hpp>

#include "MainWindow.hpp"

using json = nlohmann::json;

static void createChannelView();

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
    createFakes { new QAction("(DEBUG) Create Fakes", this) },
    logger { spdlog::stdout_color_mt("MainWindow") }
{
    // Will most likely remove this.
    resize(1280, 720);
    
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
    // QLabel* CH0StatusLabel = new QLabel(CH0Status);
    CH0StatusBoxLayout->addWidget(CH0Status);
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
    // QLabel* CH1StatusLabel = new QLabel(CH1Status);
    CH1StatusBoxLayout->addWidget(CH1Status);
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
    testFrameLayout->addStretch(64);
    testFrameLayout->addWidget(testStatusBox, Qt::AlignCenter);
    testFrameLayout->addStretch(64);
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
    helpMenu->addAction(createFakes);

    // Now we're ready to connect all the signals.
    // QObject::connect(connectAct, &QAction::triggered, this, &MainWindow::getPortInfoPSU);
    QObject::connect(connectAct, &QAction::triggered, this, &MainWindow::readConfigurationFile);
    QObject::connect(this, &MainWindow::connect, controller, &TestController::connect);
    QObject::connect(disconnectAct, &QAction::triggered, controller, &TestController::disconnect);
    QObject::connect(startAct, &QAction::triggered, this, &MainWindow::start);
    QObject::connect(stopAct, &QAction::triggered, controller, &TestController::stop);
    QObject::connect(createFakes, &QAction::triggered, this, &MainWindow::fillFakeBarcodes);

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

    QObject::connect(controller, &TestController::distributeTimeInfo, this, &MainWindow::updateTime);

    QObject::connect(this, &MainWindow::startTest, controller, &TestController::start);

    QObject::connect(
        this, 
        &MainWindow::distributeTestConfiguration, 
        controller, 
        &TestController::initializeTestConfiguration
    );

    QObject::connect(this, &MainWindow::fillFakeBarcodes, CH0Model, &CollectionModel::createFakeBarcodes);
    QObject::connect(this, &MainWindow::fillFakeBarcodes, CH1Model, &CollectionModel::createFakeBarcodes);

    QObject::connect(controller, &TestController::connected, this, &MainWindow::hasConnected);
    QObject::connect(controller, &TestController::disconnected, this, &MainWindow::hasDisconnected);
}

MainWindow::~MainWindow()
{}

void MainWindow::hasConnected()
{
    this->connectionStatus->setText("CONNECTED");
}

void MainWindow::hasDisconnected()
{
    this->connectionStatus->setText("DISCONNECTED");
}

void MainWindow::start()
{
    logger->debug("Getting to emit startTest");
    this->timeStarted->setText(QTime::currentTime().toString());
    this->timeElapsed->setText("N/A");
    this->timeRemaining->setText("N/A");
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

    auto filename = QFileDialog::getOpenFileName(this, "Configuration File");
    logger->debug(filename.toStdString());

    
}

void MainWindow::openHelp()
{}

void MainWindow::openGitHub()
{}

void MainWindow::about()
{}

void MainWindow::updateTime(std::string elapsedTime, std::string remainingTime)
{
    this->timeElapsed->setText(elapsedTime.c_str());
    this->timeRemaining->setText(remainingTime.c_str());
}

void MainWindow::alert(std::string message)
{
    QMessageBox msg(this);
    msg.setText(message.c_str());
    msg.exec();
}

void MainWindow::readConfigurationFile()
{
    auto filename_qstr = QFileDialog::getOpenFileName(this, "Configuration File");
    auto file = filename_qstr.toStdString();

    bool ok = false;
    QStringList testTypesAvailable;
    testTypesAvailable << "Normal" << "Reverse";
    QString item = QInputDialog::getItem(
        this, 
        "Get Test Type", 
        "Test Type:", 
        testTypesAvailable, 
        0, 
        false, 
        &ok, 
        Qt::MSWindowsFixedSizeDialogHint
    );

    json config;
    try
    {
        std::ifstream in_file(file);
        in_file >> config;
    }
    catch(const std::exception& e)
    {
        logger->error("Unable to parse file. Returning");
        return;
    }

    try
    {
        auto port = config["port"]["psu"]["port"].get<std::string>();
        auto baud_rate = config["port"]["psu"]["baud_rate"].get<std::string>();
        auto data_bit = config["port"]["psu"]["data_bit"].get<std::string>();
        auto stop_bit = config["port"]["psu"]["stop_bit"].get<std::string>();
        auto parity = config["port"]["psu"]["parity"].get<std::string>();
        auto lbusaddress = config["port"]["psu"]["lbusaddress"].get<std::string>();

        msu_smdt::Port arg_port = {
            port,
            baud_rate,
            data_bit,
            stop_bit,
            parity,
            lbusaddress
        };

        this->PSUPort = std::move(arg_port);
    }
    catch (std::exception & ex)
    {
        logger->error("Cannot obtain PSU port information");
    }

    try
    {
        auto port = config["port"]["hw"]["port"].get<std::string>();
        auto baud_rate = config["port"]["hw"]["baud_rate"].get<std::string>();
        auto data_bit = config["port"]["hw"]["data_bit"].get<std::string>();
        auto stop_bit = config["port"]["hw"]["stop_bit"].get<std::string>();
        auto parity = config["port"]["hw"]["parity"].get<std::string>();
        auto lbusaddress = config["port"]["hw"]["lbusaddress"].get<std::string>();

        this->HWPort = {
            port,
            baud_rate,
            data_bit,
            stop_bit,
            parity,
            lbusaddress
        };
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot obtain HW port information");
    }

    try
    {
        auto seconds_per_tube = config["test"]["seconds_per_tube"].get<int>();
        auto tubes_per_channel = config["test"]["tubes_per_channel"].get<int>();
        auto time_for_testing_voltage = config["test"]["time_for_testing_voltage"].get<int>();

        this->parameters = {
            seconds_per_tube,
            tubes_per_channel,
            time_for_testing_voltage
        };

        controller->setTestingParameters(parameters);
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot obtain test parameters from file");
    }

    TestConfiguration normalConfig;
    TestConfiguration reverseConfig;
    try
    {
        auto test_voltage = config["test"]["normal"]["test_voltage"].get<int>();
        auto current_limit = config["test"]["normal"]["current_limit"].get<int>();
        auto max_voltage = config["test"]["normal"]["max_voltage"].get<int>();
        auto ramp_up_rate = config["test"]["normal"]["ramp_up_rate"].get<int>();
        auto ramp_down_rate = config["test"]["normal"]["ramp_down_rate"].get<int>();
        auto over_current_limit = config["test"]["normal"]["over_current_limit"].get<int>();
        auto power_down_method = config["test"]["normal"]["power_down_method"].get<int>();

        normalConfig = {
            test_voltage,
            current_limit,
            max_voltage,
            ramp_up_rate,
            ramp_down_rate,
            over_current_limit,
            power_down_method,
        };

        test_voltage = config["test"]["reverse"]["test_voltage"].get<int>();
        current_limit = config["test"]["reverse"]["current_limit"].get<int>();
        max_voltage = config["test"]["reverse"]["max_voltage"].get<int>();
        ramp_up_rate = config["test"]["reverse"]["ramp_up_rate"].get<int>();
        ramp_down_rate = config["test"]["reverse"]["ramp_down_rate"].get<int>();
        over_current_limit = config["test"]["reverse"]["over_current_limit"].get<int>();
        power_down_method = config["test"]["reverse"]["power_down_method"].get<int>();

        reverseConfig = {
            test_voltage,
            current_limit,
            max_voltage,
            ramp_up_rate,
            ramp_down_rate,
            over_current_limit,
            power_down_method,
        };
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot obtain test initial conditions from file");
    }

    logger->debug("Attempting to connect");

    emit connect(PSUPort);

    if (ok && !item.isEmpty())
        emit distributeTestConfiguration(normalConfig, reverseConfig);
}