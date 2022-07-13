#include <spdlog/sinks/stdout_color_sinks.h>

#include "ChannelWidget.hpp"

constexpr char* msg = "ON | RUP | RDWN | OVC | OVV | UNV | MAXV | TRIP | OVP | OVT | DIS | KILL | ILK | NOCAL";

ChannelWidget::ChannelWidget(QObject* parent, int channel, TestParameters parameters):
    QObject(parent),
    polarity { "*" };
    channelStatus { new QLabel(msg, this) },
    dataView { new QTableView(this) },
    this->parameters { parameters };
{
    if (channel && channel < 4)
    {
        name = "CH" + std::to_string(channel) + " (" + polarity + ")";
        channelDataBox = new QGroupBox((name + " - Data"), this);
        channelStatusBox = new QGroupBox((name + " - Status"), this);
        dataModel = new CollectionModel(this, channel, parameters);
    }
    else
    {
        emit alert("Invalid Channel");
    }
}

ChannelWidget::~ChannelWidget()
{}

void ChannelWidget::receiveChannelPolarity(int polarity)
{
    if (this->polarity == "*")
    {
        if (polarity < 0)
        {
            this->polarity = "-";
        }
        else if (polarity > 0)
        {
            this->polarity = "+";
        }
        else
        {
            this->polarity = "*";
        }

        name = "CH" + std::to_string(channel) + " (" + this->polarity + ")";
    }
}

void ChannelWidget::receiveTubeDataPacket(TubeData data)
{}
