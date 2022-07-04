#pragma once

#include <QThread>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <psu/Port.hpp>
#include <psu/HVInterface.hpp>

void TestControlOfPowerSupply();