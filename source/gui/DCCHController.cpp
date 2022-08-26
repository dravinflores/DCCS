#include <spdlog/sinks/stdout_color_sinks.h>

#include "DCCHController.hpp"

#ifndef Q_OS_WIN

DCCHController::DCCHController(QObject* parent):
    QObject(parent),
    port { new QSerialPort },
    buf(4, '*')
{
    try
    {
        logger = spdlog::stdout_color_mt("Serial");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        logger = spdlog::get("Serial");
    }
}

DCCHController::DCCHController(QObject* parent, msu_smdt::Port DCCHPort):
    QObject(parent),
    port { new QSerialPort },
    buf(4, '*')
{
    // port->setPortName(QString::fromStdString(DCCHPort.port));
    // port->setBaudRate(std::stoi(DCCHPort.baud_rate));
    // port->setDataBits(QSerialPort::Data8);

    try
    {
        logger = spdlog::stdout_color_mt("Serial");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        logger = spdlog::get("Serial");
    }

    setPort(DCCHPort);
}

DCCHController::~DCCHController()
{
    port->close();
}

void DCCHController::setPort(msu_smdt::Port DCCHPort)
{
    port->setPortName(QString::fromStdString(DCCHPort.port));
    port->setBaudRate(std::stoi(DCCHPort.baud_rate));
    port->setDataBits(QSerialPort::Data8);

    buf[0] = '{';
    buf[3] = '}';

    if (!port->open(QIODeviceBase::ReadWrite))
    {
        logger->error("Cannot connect to DCCH Board [FATAL]: {}", port->errorString().toStdString());
    }
}

void DCCHController::connectTube(int tube)
{
    buf[1] = (char) tube;
    buf[2] = (char) 1;

    int r = port->write(buf.data(), buf.size());

    while (port->waitForBytesWritten());

    if (!r)
        logger->error("Cannot enable tube through Serial. Error: {}", port->errorString().toStdString());

    logger->debug("Connected tube {}", tube);
}

void DCCHController::disconnectTube(int tube)
{
    buf[1] = (char) tube;
    buf[2] = (char) 0;

    int r = port->write(buf.data(), buf.size());

    while (port->waitForBytesWritten());

    if (!r)
    {
        logger->error("Cannot disable tube through Serial. Error: {}", port->errorString().toStdString());
    }
}

#else
DCCHController::DCCHController(msu_smdt::Port DCCHPort):
    connected { false },
    buf(4, '*')
{
    try
    {
        logger = spdlog::stdout_color_mt("Serial");
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        logger = spdlog::get("Serial");
    }

    handle = CreateFileA(
        static_cast<LPCSTR>(DCCHPort.port.c_str()),
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE)
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
            logger->error("Port {} not available", DCCHPort.port);
        else
            logger->error("Unknown Error");
    }

    DCB parameters { 0 };

    if (!GetCommState(handle, &parameters))
    {
        logger->error("Cannot obtain parameters");
    }
    else
    {
        parameters.BaudRate = CBR_9600;
        parameters.ByteSize = 8;
        parameters.StopBits = ONESTOPBIT;
        parameters.Parity = NOPARITY;
        parameters.fDtrControl = DTR_CONTROL_ENABLE;

        if (!SetCommState(handle, &parameters))
        {
            logger->error("Cannot set parameters");
        }
        else
        {
            this->connected = true;
            PurgeComm(handle, PURGE_RXCLEAR | PURGE_TXCLEAR);
            Sleep(2000);
        }
    }

    buf[0] = '{';
    buf[3] = '}';
}

DCCHController::~DCCHController()
{
    if (connected)
        CloseHandle(handle);
}

void DCCHController::connectTube(int tube)
{
    buf[1] = (char) tube;
    buf[2] = (char) 1;

    DWORD bytesSent = 0;

    if (!WriteFile(handle, (void*) buf.data(), buf.size(), &bytesSent, 0))
        ClearCommError(handle, &error, &status);
}

void DCCHController::disconnectTube(int tube)
{
    buf[1] = (char) tube;
    buf[2] = (char) 0;

    DWORD bytesSent = 0;

    if (!WriteFile(handle, (void*) buf.data(), buf.size(), &bytesSent, 0))
        ClearCommError(handle, &error, &status);
}

#endif