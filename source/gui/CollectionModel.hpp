// CollectionModel.hpp

#pragma once

#include <string>
#include <vector>

#include <QAbstractTableModel>

#include "TestController.hpp"
#include "TestInfo.hpp"

class CollectionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CollectionModel(QObject* parent, int channel, TestParameters parameters);
    ~CollectionModel();

    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);

public slots:
    void receiveChannelPolarity(int polarity);
    void receiveTubeDataPacket(TubeData data);

private:
    int channel;
    int polarity;
    TestParameters parameters;
    std::vector<TubeData> internalData;
    std::vector<std::string> barcodes;
};