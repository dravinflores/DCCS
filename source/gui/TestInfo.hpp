// TestInfo.hpp

#pragma once

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
    unsigned long status { 0 };
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
    int maxVoltage { 0 };
    int rampUpRate { 0 };
    int rampDownRate { 0 };
    int overCurrentLimit { 0 };
    int powerDownMethod { 0 };
};