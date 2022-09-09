#pragma once

#include <memory>
#include <utility>

#include <QObject>
#include <QSerialPort>

#include <spdlog/spdlog.h>

#include <psu/Port.hpp>
#include "TestInfo.hpp"

#ifndef Q_OS_WIN

class DCCHController : public QObject
{
    Q_OBJECT

public:
    DCCHController(QObject* parent = nullptr);
    DCCHController(QObject* parent, msu_smdt::Port DCCHPort);
    ~DCCHController();

    void setPort(msu_smdt::Port DCCHPort);

public slots:
    void connectTube(int tube);
    void disconnectTube(int tube);

private:
    QSerialPort* port;
    std::vector<char> buf;
    std::shared_ptr<spdlog::logger> logger;
};

#else

#include <Windows.h>

class DCCHController : public QObject
{
public:
    DCCHController(msu_smdt::Port DCCHPort);
    ~DCCHController();

    void connectTube(int tube);
    void disconnectTube(int tube);

private:
    bool connected;

    HANDLE handle;
    COMSTAT status;
    DWORD error;    

    std::vector<char> buf;
    std::shared_ptr<spdlog::logger> logger;
};
#endif