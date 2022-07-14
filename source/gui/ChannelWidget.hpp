#pragma once

#include <string>
#include <memory>
#include <utility>

#include <QLabel>
#include <QWidget>
#include <QGroupBox>
#include <QGridLayout>

#include <QTableView>

#include <spdlog/spdlog.h>

#include "CollectionModel.hpp"
// #include "TestController.hpp"
#include "TestInfo.hpp"

class ChannelWidget : public QWidget
{
    Q_OBJECT

public:
    ChannelWidget(QWidget* parent = nullptr, TestParameters parameters = TestParameters());
    ~ChannelWidget();

    ChannelWidget(const ChannelWidget&) = delete;
    ChannelWidget(ChannelWidget&&) = delete;

    ChannelWidget& operator=(const ChannelWidget&) = delete;
    ChannelWidget& operator=(ChannelWidget&&) = delete;

    void setChannel(int channel);

    void receiveChannelPolarity(int polarity);
    void receiveTubeDataPacket(TubeData data);
    void fillWithFakeBarcodes();

signals:
    void issueAlert(std::string msg);

private:
    int channel;
    std::string name;
    std::string polarity;

    QLabel* channelStatus;

    QGroupBox* channelDataBox;
    QGroupBox* channelStatusBox;

    QTableView* dataView;
    CollectionModel* dataModel;

    TestParameters parameters;

    std::shared_ptr<spdlog::logger> logger;
};