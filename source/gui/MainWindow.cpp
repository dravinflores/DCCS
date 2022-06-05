#include <gui/MainWindow.hpp>

#include <psu/Port.hpp>
#include <psu/PSUController.hpp>

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent)
{
    logger = spdlog::get("gui_logger");
    logger->debug("MainWindow initialized");

    msu_smdt::Port fake_com_port_connection {
        .port           = "COM4",
        .baud_rate      = "9600",
        .data_bit       = "8",
        .stop_bit       = "0",
        .parity         = "0",
        .lbusaddress    = "0"
    };
        
    PSUController psuobj(true, "");
    psuobj.connect(fake_com_port_connection);
    psuobj.pretendTest();
}

MainWindow::~MainWindow()
{
    logger->debug("MainWindow deinitialized");
}
