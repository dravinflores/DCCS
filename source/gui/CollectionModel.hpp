// CollectionModel.hpp

#pragma once

#include <map>
#include <string>
#include <vector>

#include <QAbstractTableModel>

#include "TestController.hpp"
#include "TestInfo.hpp"

class CollectionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit CollectionModel(QObject* parent, TestParameters parameters);
    ~CollectionModel();

    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);

    void setChannel(int channel);

    void setTestParameters(TestParameters parameters);

    void storeTubeDataPacket(TubeData data);
    void createFakeBarcodes();

    std::map<std::string, TubeData> getDataForCSV();

private:
    int channel;
    TestParameters parameters;
    std::vector<TubeData> internalData;
    std::vector<std::string> barcodes;
};