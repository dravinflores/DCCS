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

// #include <gui/MainWindow.hpp>

#define NO_GUI
#ifdef NO_GUI
    #include "test/manual/test.hpp"
#endif 

int main(int argc, char** argv)
{
    // spdlog::set_level(spdlog::level::debug);
    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%=16n] %^[%=8l]%$ %v");

#ifndef NO_GUI
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    MainWindow main_window;
    main_window.show();

    spdlog::drop_all();

    return app.exec();
#else
    TestControlOfPowerSupply();
#endif
}