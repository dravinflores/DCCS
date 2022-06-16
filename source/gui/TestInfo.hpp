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