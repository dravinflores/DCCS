#include "test.hpp"

void TestControlOfPowerSupply()
{
    auto logger = spdlog::stdout_color_mt("Main");

    HVInterface interface;
    msu_smdt::Port port = {
        .port = "COM3",
        .baud_rate = "9600",
        .data_bit = "8",
        .stop_bit = "0",
        .parity = "0",
        .lbusaddress = "0"
    };
    interface.connectToPSU(port);

    interface.clearAlarm();
    interface.setInterlock(true);

    interface.setParametersFloat("VSet", 15.00f, {0, 1, 2, 3});
    interface.setParametersFloat("ISet", 2.000f, {0, 1, 2, 3});
    interface.setParametersFloat("MaxV", 100.0f, {0, 1, 2, 3});
    interface.setParametersFloat("RUp", 15.00f, {0, 1, 2, 3});
    interface.setParametersFloat("RDwn", 15.00f, {0, 1, 2, 3});
    interface.setParametersFloat("Trip", 1000.0f, {0, 1, 2, 3});
    interface.setParametersLong("PDwn", 0, {0, 1, 2, 3});

    interface.setParametersLong("Pw", 1, {0, 1});

    for (int i = 0; i < 15; ++i)
    {
        auto voltage = interface.getParametersFloat("VMon", {0, 1});
        auto current = interface.getParametersFloat("IMonH", {0, 1});

        logger->info(
            "\tCH0: ({} V, {} nA), CH1: ({} V, {} nA)", 
            voltage[0], 
            current[0]*1000,
            voltage[1], 
            current[1]*1000
        );
        QThread::sleep(1);
    }

    interface.setParametersLong("Pw", 0, {0, 1});
}