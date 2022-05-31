#include <gui/MainWindow.hpp>

#include <psu/com_port_struct.hpp>
#include <psu/psu.hpp>

MainWindow::MainWindow(QWidget* parent):
    QMainWindow(parent)
{
    logger = spdlog::get("gui_logger");
    logger->debug("MainWindow initialized");

    msu_smdt::com_port fake_com_port_connection {
        .port           = "COM4",
        .baud_rate      = "9600",
        .data_bit       = "8",
        .stop_bit       = "0",
        .parity         = "0",
        .lbusaddress    = "0"
    };
        
    control_psu psuobj(fake_com_port_connection);
    psuobj.pretend_start();
}

MainWindow::~MainWindow()
{
    logger->debug("MainWindow deinitialized");
}
