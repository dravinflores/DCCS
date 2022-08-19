#include <spdlog/sinks/stdout_color_sinks.h>

#include "DCCHController.hpp"

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
