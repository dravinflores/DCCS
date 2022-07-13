#pragma once

#include <string>
#include <memory>
#include <utility>

#include <QLabel>
#include <QWidget>
#include <QGroupBox>
#include <QGridLayout>

#include <spdlog/spdlog.h>

#include "TestController.hpp"
#include "TestInfo.hpp"

class ChannelWidget : public QWidget
{
    Q_OBJECT

public:
    ChannelWidget(QObject* parent, int channel, TestParameters parameters);
    ~ChannelWidget();

    ChannelWidget(const ChannelWidget&) = delete;
    ChannelWidget(ChannelWidget&&) = delete;

    ChannelWidget& operator=(const ChannelWidget&) = delete;
    ChannelWidget& operator=(ChannelWidget&&) = delete;

public slots:
    void receiveChannelPolarity(int polarity);
    void receiveTubeDataPacket(TubeData data);

signals:
    void alert(std::string msg);

private:
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