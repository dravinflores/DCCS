// TestInfo.hpp

#pragma once

#include <string>

struct TubeData
{
    int index { -1 };
    int channel { -1 };
    bool isActive { false };
    float current { -1.00 };
    float voltage { -1.00 };
};

struct ChannelStatus
{
    int channel { -1 };
    std::string status { "" };
};

enum class TestMode
{
    Normal,
    Reverse
};

struct TestParameters
{
    int secondsPerTube { 1 };
    int tubesPerChannel { 32 };
    int timeForTestingVoltage { 1 };
};

struct TestConfiguration
{
    int testVoltage { 0 };
    int currentLimit { 0 };
    int maxVoltage { 1 };
    int rampUpRate { 1 };
    int rampDownRate { 1 };
    int overCurrentLimit { 0 };
    int powerDownMethod { 0 };
};