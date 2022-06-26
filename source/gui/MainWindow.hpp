// The Application looks as the following:
// +------------------------------------------------------------------------------------------------------------------+
// |    CH0 - Data                                                CH1 - Data                                          |
// |    +-------+----------+--------------+-------------+         +-------+----------+--------------+-------------+   |
// |    | Index | Barcode  | Current [nA] | Voltage [V] |         | Index | Barcode  | Current [nA] | Voltage [V] |   |
// |    |-------|----------|--------------|-------------|         |-------|----------|--------------|-------------|   |
// |    | 0     | MSU00000 | 0.00         | 0.00        |         | 0     | MSU00000 | 0.00         | 0.00        |   |
// |    | 1     | MSU00000 | 0.00         | 0.00        |         | 1     | MSU00000 | 0.00         | 0.00        |   |
// |    | 2     | MSU00000 | 0.00         | 0.00        |         | 2     | MSU00000 | 0.00         | 0.00        |   |
// |    +-------+----------+--------------+-------------+         +-------+----------+--------------+-------------+   |
// |                                                                                                                  |
// |    CH0 - Status                                              CH1 - Status                                        |
// |    +-----------------------------------------------+         +-----------------------------------------------+   |
// |    |                                               |         |                                               |   |
// |    +-----------------------------------------------+         +-----------------------------------------------+   |
// |                                                                                                                  |
// |                                                    Test Status                                                   |
// |                                           +---------------------------+                                          |
// |                                           |  Time Started:    00:00   |                                          |
// |                                           |  Time Elapsed:    0 min   |                                          |
// |                                           |  Time Remaining:  0 min   |                                          |
// |                                           +---------------------------+                                          |
// +------------------------------------------------------------------------------------------------------------------+

#pragma once

#include <string>
#include <memory>
#include <utility>

#include <QMenu>
#include <QLabel>
#include <QFrame>
#include <QAction>
#include <QGroupBox>
#include <QTableView>
#include <QMainWindow>
#include <QActionGroup>

#include <spdlog/spdlog.h>

#include "TestInfo.hpp"
#include "TestController.hpp"
#include "CollectionModel.hpp"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

signals:
    void connect(msu_smdt::Port port);
    void startTest(std::vector<int> channels);

private slots:
    void start();
    void getPortInfoPSU();
    void openHelp();
    void openGitHub();
    void about();

    void update();

private:
    msu_smdt::Port PSUPort;
    msu_smdt::Port HWPort;

    TestParameters parameters;
    TestController* controller;

    CollectionModel* CH0Model;
    CollectionModel* CH1Model;

    QTableView* CH0View;
    QTableView* CH1View;

    QLabel* CH0Status;
    QLabel* CH1Status;
    QLabel* connectionStatus;
    QLabel* timeStarted;
    QLabel* timeElapsed;
    QLabel* timeRemaining;

    QFrame* channelFrame;
    QFrame* testStatusFrame;

    QGroupBox* CH0DataBox;
    QGroupBox* CH1DataBox;
    QGroupBox* CH0StatusBox;
    QGroupBox* CH1StatusBox;
    QGroupBox* testStatusBox;

    QMenu* connectMenu;
    QMenu* testMenu;
    QMenu* helpMenu;

    QAction* connectAct;
    QAction* disconnectAct;
    QAction* startAct;
    QAction* stopAct;
    QAction* openHelpAct;
    QAction* openGitHubAct;
    QAction* aboutAct;

    std::shared_ptr<spdlog::logger> logger;
};