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
    // std::string executable_dir = QDir::currentPath().toStdString();
    // std::string log_file = executable_dir + "/log/log.txt";

    std::string log_file = "log/log.txt";

    // Here we will create our central sink.
    auto central_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file);

    // Now we can create the separate loggers.
    auto main_logger = std::make_shared<spdlog::logger>("main_logger", central_sink);
    auto gui_logger = std::make_shared<spdlog::logger>("gui_logger", central_sink);
    auto psu_logger = std::make_shared<spdlog::logger>("psu_logger", central_sink);
    auto hvlib_logger = std::make_shared<spdlog::logger>("hvlib_logger", central_sink);

    // Register these loggers.
    spdlog::register_logger(main_logger);
    spdlog::register_logger(gui_logger);
    spdlog::register_logger(psu_logger);
    spdlog::register_logger(hvlib_logger);

    // Adjust the level.
    main_logger->set_level(spdlog::level::debug);
    gui_logger->set_level(spdlog::level::debug);
    psu_logger->set_level(spdlog::level::debug);
    hvlib_logger->set_level(spdlog::level::debug);

    // We want to immediately write every debug message;
    main_logger->flush_on(spdlog::level::debug);
    gui_logger->flush_on(spdlog::level::debug);
    psu_logger->flush_on(spdlog::level::debug);
    hvlib_logger->flush_on(spdlog::level::debug);

    QApplication app(argc, argv);
    MainWindow main_window;
    main_window.show();

    spdlog::drop_all();

    return app.exec();
}