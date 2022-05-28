#pragma once

#include <iostream>
#include <memory>

#include <spdlog/spdlog.h>

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    std::shared_ptr<spdlog::logger> logger;
};