#include "MainWindow.hpp"

#include <iostream>
#include <fstream>
#include <exception>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <nlohmann/json.hpp>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>

#include <QMessageBox>
#include <QErrorMessage>

#include <QInputDialog>

using json = nlohmann::json;

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    hasStarted { false },
    controller { new TestController(this) },
    channelWidgetContainer { new QWidget },
    controlPanelWidget { new ControlPanelWidget },
    channelWidgetLeft { new ChannelWidget },
    channelWidgetRight { new ChannelWidget },
    testStatusWidget { new TestStatusWidget }
{
    try
    {
        logger = spdlog::stdout_color_mt("MainWindow");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        logger = spdlog::get("MainWindow");
    }

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

    /*
    channelWidgetLeft->setChannel(0);
    channelWidgetRight->setChannel(1);

    channelWidgetLeft->receiveChannelPolarity(-1);
    channelWidgetRight->receiveChannelPolarity(+1);
    */

    QObject::connect(
        controlPanelWidget, 
        &ControlPanelWidget::requestToConnect, 
        this, 
        &MainWindow::receiveRequestToConnect
    );


    QObject::connect(
        controlPanelWidget, 
        &ControlPanelWidget::requestToStart, 
        this, 
        &MainWindow::receiveRequestToStart
    );

    QObject::connect(
        this,
        &MainWindow::connectionStatusChanged,
        controlPanelWidget,
        &ControlPanelWidget::connectionChanged
    );

    QObject::connect(
        this,
        &MainWindow::executionStatusChanged,
        controlPanelWidget,
        &ControlPanelWidget::executionChanged
    );

    QObject::connect(
        this,
        &MainWindow::start,
        controller,
        &TestController::start
    );

    QObject::connect(
        this,
        &MainWindow::stop,
        controller,
        &TestController::stop
    );

    // Now we need to connect the data.
    QObject::connect(
        controller,
        &TestController::distributeTubeDataPacket,
        channelWidgetLeft,
        &ChannelWidget::receiveTubeDataPacket
    );

    QObject::connect(
        controller,
        &TestController::distributeTubeDataPacket,
        channelWidgetRight,
        &ChannelWidget::receiveTubeDataPacket
    );

    QObject::connect(
        controller,
        &TestController::distributeChannelPolarity,
        channelWidgetLeft,
        &ChannelWidget::receiveChannelPolarity
    );

    QObject::connect(
        controller,
        &TestController::distributeChannelPolarity,
        channelWidgetRight,
        &ChannelWidget::receiveChannelPolarity
    );

    QObject::connect(
        controller,
        &TestController::distributeChannelStatus,
        channelWidgetLeft,
        &ChannelWidget::receiveChannelStatus
    );

    QObject::connect(
        controller,
        &TestController::distributeChannelStatus,
        channelWidgetRight,
        &ChannelWidget::receiveChannelStatus
    );

    QObject::connect(
        controller,
        &TestController::distributeTimeInfo,
        [this](std::string time) {
            testStatusWidget->receiveTimeRemaining(time);
            // testStatusWidget->update();
        }
    );

    QObject::connect(
        this,
        &MainWindow::start,
        [this]() {
            testStatusWidget->startTime();
            testStatusWidget->update();
        }
    );

    QObject::connect(
        this,
        &MainWindow::stop,
        [this]() {
            testStatusWidget->stopTime();
        }
    );

    QObject::connect(
        controller,
        &TestController::finished,
        [this]() {
            // testStatusWidget->stopTime();
            emit stop();
            controlPanelWidget->executionChanged(false);
            hasStarted = false;
        }
    );

    /*
    QObject::connect(
        controller,
        &TestController::finished,
        [this]() {
            controlPanelWidget->executionChanged(false);
            hasStarted = false;
            // receiveRequestToStart();
        }
    );
    */
}

MainWindow::~MainWindow()
{}

bool MainWindow::readInTestSettings()
{
    auto file = QFileDialog::getOpenFileName(this, "Configuration File (JSON)").toStdString();
    bool no_config = file.empty();

    if (no_config)
        return false;

    json config;
    try
    {
        std::ifstream in_file(file);
        in_file >> config;
    }
    catch(const std::exception& e)
    {
        logger->error("Unable to parse file. Returning");
        return false;
    }

    msu_smdt::Port HWPort;
    msu_smdt::Port PSUPort;

    try
    {
        auto port = config["port"]["psu"]["port"].get<std::string>();
        auto baud_rate = config["port"]["psu"]["baud_rate"].get<std::string>();
        auto data_bit = config["port"]["psu"]["data_bit"].get<std::string>();
        auto stop_bit = config["port"]["psu"]["stop_bit"].get<std::string>();
        auto parity = config["port"]["psu"]["parity"].get<std::string>();
        auto lbusaddress = config["port"]["psu"]["lbusaddress"].get<std::string>();

        PSUPort = {
            port,
            baud_rate,
            data_bit,
            stop_bit,
            parity,
            lbusaddress
        };
    }
    catch (std::exception & ex)
    {
        logger->error("Cannot obtain PSU port information");
        return false;
    }

    try
    {
        auto port = config["port"]["hw"]["port"].get<std::string>();
        auto baud_rate = config["port"]["hw"]["baud_rate"].get<std::string>();
        auto data_bit = config["port"]["hw"]["data_bit"].get<std::string>();
        auto stop_bit = config["port"]["hw"]["stop_bit"].get<std::string>();
        auto parity = config["port"]["hw"]["parity"].get<std::string>();
        auto lbusaddress = config["port"]["hw"]["lbusaddress"].get<std::string>();

        HWPort = {
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
        return false;
    }

    TestParameters parameters;
    try
    {
        auto seconds_per_tube = config["test"]["seconds_per_tube"].get<int>();
        auto tubes_per_channel = config["test"]["tubes_per_channel"].get<int>();
        auto time_for_testing_voltage = config["test"]["time_for_testing_voltage"].get<int>();

        parameters = {
            seconds_per_tube,
            tubes_per_channel,
            time_for_testing_voltage
        };
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot obtain test parameters from file");
        return false;
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
        return false;
    }

    try
    {
        std::vector<int> normalChannels = config["channels"]["normal"].get<std::vector<int>>();
        std::vector<int> reverseChannels = config["channels"]["reverse"].get<std::vector<int>>();

        this->normalChannels = normalChannels;
        this->reverseChannels = reverseChannels;
    }
    catch (std::exception& ex)
    {
        logger->error("Cannot obtain Normal and Reverse channels");
        return false;
    }

    // this->PSUPort = std::move(PSUPort);
    // this->HWPort = std::move(HWPort);
    // this->parameters = std::move(parameters);
    // this->normalConfig = std::move(normalConfig);
    // this->reverseConfig = std::move(reverseConfig);

    this->PSUPort = PSUPort;
    this->HWPort = HWPort;
    this->parameters = parameters;
    this->normalConfig = normalConfig;
    this->reverseConfig = reverseConfig;

    // bool TestWithAllChannels = config["experimental"]["test_with_all_channels"].get<bool>();

    return true;
}

void MainWindow::raiseAlert(std::string msg)
{
    QMessageBox messageBox;
    messageBox.setText(msg.c_str());
    messageBox.exec();
}


void MainWindow::receiveRequestToConnect()
{
    if (controller->checkConnection() && !hasStarted)
    {
        controller->disconnect();
        emit connectionStatusChanged(false);
        testStatusWidget->updateConnectionStatus(false);
    }
    else if (hasStarted)
    {
        raiseAlert("Cannot disconnect while testing");
    }
    else
    {
        bool r = readInTestSettings();
        controller->connect(PSUPort, HWPort);
        controller->setTestParameters(this->parameters);
        channelWidgetLeft->setTestParameters(this->parameters);
        channelWidgetRight->setTestParameters(this->parameters);

        controller->initializeTestConfiguration(this->normalConfig, this->reverseConfig);
        emit connectionStatusChanged(true);
        testStatusWidget->updateConnectionStatus(true);
    }
}

void MainWindow::receiveRequestToStart()
{
    if (hasStarted)
    {
        emit stop();
        controlPanelWidget->executionChanged(false);
        hasStarted = false;
    }
    else
    {
        hasStarted = true;
        bool mode = false;
        
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

        std::vector<int> v;

        if (ok && !item.isEmpty())
        {
            if (item == "Normal")
            {
                mode = true;
                v = normalChannels;
            }
            else
            {
                mode = false;
                v = reverseChannels;
            }

            channelWidgetLeft->setChannel(v[0]);
            channelWidgetRight->setChannel(v[1]);

            emit executionStatusChanged(true);
            emit start(v, mode);
        }
    }
}

void MainWindow::alertUser(std::string msg)
{
    QErrorMessage messageBox;
    messageBox.showMessage(msg.c_str());
}
