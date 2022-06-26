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
#include <QDir>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <gui/MainWindow.hpp>

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::debug);

    QApplication app(argc, argv);
    MainWindow main_window;
    main_window.show();

    spdlog::drop_all();

    return app.exec();
}