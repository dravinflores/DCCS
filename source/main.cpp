/***********************************************************************************************************************
 *  File:           main.cpp
 *  Author(s):      Dravin Flores <dravinflores@gmail.com>
 *  Date Created:   16 December, 2021
 * 
 *  Purpose:        This is the main entry of the program.
 * 
 *  Known Issues:   
 * 
 *  Workarounds:    
 * 
 *  Updates:
 **********************************************************************************************************************/

#include <QApplication>
#include <QStyleFactory>
#include <QDir>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define TEST_GUI
#ifdef TEST_GUI
    #include <QWidget>
    #include <QVBoxLayout>

    #include <gui/ChannelWidget.hpp>
    #include <gui/TestStatusWidget.hpp>
    #include <gui/ControlPanelWidget.hpp>
    #include <gui/TestInfo.hpp>
#endif

#include <gui/MainWindow.hpp>

#ifdef NO_GUI
    #include "test/manual/test.hpp"
#endif 

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::debug);
    // spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%=16n] %^[%=8l]%$ %v");

/*
#ifdef TEST_GUI
    TestParameters params;

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    QWidget mainWidget;
    ChannelWidget channelWidget(nullptr, 0, params);
    TestStatusWidget testStatusWidget(nullptr);
    ControlPanelWidget controlPanelWidget(nullptr);

    // QGridLayout* layout = new QGridLayout;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(&controlPanelWidget);
    layout->addWidget(&channelWidget);
    layout->addWidget(&testStatusWidget);
    mainWidget.setLayout(layout);

    mainWidget.show();

    spdlog::drop_all();

    return app.exec();
#else
    // TestControlOfPowerSupply();
    TestPSUController();
#endif
*/

    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    MainWindow window;
    window.show();
    return app.exec();
}