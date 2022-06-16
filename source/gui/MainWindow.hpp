#pragma once

#include <QTableView>
#include <QMainWindow>

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
    void startTest(std::vector<int> channels);
    void stop();

private:
    TestParameters parameters;
    CollectionModel* collectionModel;
    QTableView* tableView;
    TestController* controller;
};