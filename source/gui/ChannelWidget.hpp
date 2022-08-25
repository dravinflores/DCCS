#pragma once

#include <map>
#include <string>
#include <memory>
#include <utility>

#include <QEvent>
#include <QLabel>
#include <QWidget>
#include <QKeyEvent>
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

    void setTestParameters(TestParameters params);

    std::map<std::string, TubeData> getDataForCSV();

public slots:
    void receiveChannelPolarity(int channel, int polarity);
    void receiveChannelStatus(int channel, std::string status);
    void receiveTubeDataPacket(TubeData data);

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