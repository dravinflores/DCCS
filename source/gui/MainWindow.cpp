// MainWindow.cpp

#include <QWidget>
#include <QVBoxLayout>
#include <QHeaderView>

#include <psu/Port.hpp>
#include <psu/PSUController.hpp>

#include "MainWindow.hpp"

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent),
    parameters {
        .secondsPerTube { 4 },
        .tubesPerChannel { 16 },
        .timeForTestingVoltage { 1 },
    },
    collectionModel { new CollectionModel(this, 0, parameters) },
    tableView { new QTableView(this) },
    controller { new TestController(this) }
{
    resize(800, 600);
    tableView->setModel(collectionModel);
    tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    tableView->setGridStyle(Qt::NoPen);
    tableView->setAlternatingRowColors(true);

    QWidget* centralWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(tableView);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    msu_smdt::Port fake_com_port_connection {
        .port           = "COM4",
        .baud_rate      = "9600",
        .data_bit       = "8",
        .stop_bit       = "0",
        .parity         = "0",
        .lbusaddress    = "0"
    };

    controller->setTestingParameters(parameters);
    controller->connect(fake_com_port_connection);

    connect(
        controller, 
        &TestController::completeChannelPolarityRequest, 
        collectionModel, 
        &CollectionModel::receiveChannelPolarity
    );

    connect(
        controller,
        &TestController::distributeTubeDataPacket,
        collectionModel,
        &CollectionModel::receiveTubeDataPacket
    );

    connect(this, &MainWindow::startTest, controller, &TestController::start);

    emit startTest({0});
}

MainWindow::~MainWindow()
{}