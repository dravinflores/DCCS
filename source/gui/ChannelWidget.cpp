#include <spdlog/sinks/stdout_color_sinks.h>

#include <QHeaderView>
#include <QGridLayout>
#include <QVBoxLayout>

#include "ChannelWidget.hpp"

ChannelWidget::ChannelWidget(QWidget* parent, TestParameters parameters):
    QWidget(parent),
    polarity { "~" },
    channelStatus { new QLabel },
    channelDataBox { new QGroupBox },
    channelStatusBox { new QGroupBox },
    dataView { new QTableView },
    parameters { parameters }
{
    const char* msg = "ON | RUP | RDWN | OVC | OVV | UNV | MAXV | TRIP | OVP | OVT | DIS | KILL | ILK | NOCAL";
    channelStatus->setText(msg);
    channelStatus->setAlignment(Qt::AlignCenter);

    name = "CH_ (" + polarity + ")";

    channelDataBox->setTitle(QString::fromStdString(name + " - Data"));
    channelStatusBox->setTitle(QString::fromStdString(name + " - Status"));

    dataModel = new CollectionModel(this, parameters);

    dataView->setModel(dataModel);
    dataView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    dataView->setGridStyle(Qt::NoPen);
    dataView->setAlternatingRowColors(true);

    QGridLayout* dataBoxLayout = new QGridLayout;
    dataBoxLayout->addWidget(dataView);
    channelDataBox->setLayout(dataBoxLayout);

    QGridLayout* statusBoxLayout = new QGridLayout;
    statusBoxLayout->addWidget(channelStatus);
    channelStatusBox->setLayout(statusBoxLayout);

    QVBoxLayout* layout = new QVBoxLayout;

    layout->addWidget(channelDataBox);
    layout->addWidget(channelStatusBox);
    setLayout(layout);
}

ChannelWidget::~ChannelWidget()
{}

void ChannelWidget::setChannel(int channel)
{
    if (channel < 0 || channel > 3)
    {
        emit issueAlert("Invalid Channel");
        return;
    }

    this->channel = channel;
    this->dataModel->setChannel(channel);
    name = "CH" + std::to_string(channel) + " (" + polarity + ")";
    channelDataBox->setTitle(QString::fromStdString(name + " - Data"));
    channelStatusBox->setTitle(QString::fromStdString(name + " - Status"));
}

void ChannelWidget::setTestParameters(TestParameters params)
{
    this->parameters = params;
    dataModel->setTestParameters(params);
}

std::map<std::string, TubeData> ChannelWidget::getDataForCSV()
{
    return dataModel->getDataForCSV();
}

void ChannelWidget::receiveChannelPolarity(int channel, int polarity)
{
    if (channel != this->channel)
        return;

    if (this->polarity == "~")
    {
        if (polarity > 0)
        {
            this->polarity = "-";
        }
        else if (polarity == 0)
        {
            this->polarity = "+";
        }
        else
        {
            this->polarity = "~";
        }

        name = "CH" + std::to_string(channel) + " (" + this->polarity + ")";
        channelDataBox->setTitle(QString::fromStdString(name + " - Data"));
        channelStatusBox->setTitle(QString::fromStdString(name + " - Status"));
    }
}

void ChannelWidget::receiveChannelStatus(int channel, std::string status)
{
    if (channel != this->channel)
        return;

    channelStatus->setText(QString::fromStdString(status));
}

void ChannelWidget::receiveTubeDataPacket(TubeData data)
{
    dataModel->storeTubeDataPacket(data);
}

void ChannelWidget::receiveIntrinsicCurrent(int channel, float current)
{
    if (channel != this->channel)
        return;

    name = "CH" + std::to_string(channel) + " (" + this->polarity + ")";
    channelDataBox->setTitle(QString::fromStdString(name + " " + std::to_string(current) + "nA"));
}