// CollectionModel.cpp

#include <fmt/core.h>

#include <QColor>

#include "CollectionModel.hpp"

CollectionModel::CollectionModel(QObject* parent, int channel, TestParameters parameters):
    QAbstractTableModel(parent),
    channel { channel },
    polarity { 0 },
    parameters { parameters },
    internalData(parameters.tubesPerChannel),
    barcodes(parameters.tubesPerChannel, "")
{}

CollectionModel::~CollectionModel()
{}

int CollectionModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    // return 5;
    return 4;
}

QVariant CollectionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignCenter;

    int col = index.column();
    int row = index.row();

    // We need to check if the row exists. To be implemented later.
    bool dataExists = barcodes[row] != "";
    bool addTestingIcon = internalData[row].isActive;

    if (col == 0 && role == Qt::DisplayRole)
    {
        return row;
    }

    // We want to try coloring things
    if (dataExists && role == Qt::DecorationRole)
    {
        if (col == 1 && addTestingIcon)
        {
            return QColor("orange");
        }
    }

    if (dataExists && role == Qt::BackgroundRole)
    {
        if (col == 2 && internalData[row].current > 2.00)
            return QColor("red");
    }

    if (dataExists && role == Qt::DisplayRole)
    {
        switch (col)
        {
        case 2:
            return internalData[row].current;
        case 3:
            return internalData[row].voltage;
        case 4:
            return QVariant();
        }

    #ifdef COMMENT
        if (col == 1 && addTestingIcon)
        {
            auto str = "[ACTIVE] " + barcodes[row];
            return QString::fromStdString(str); 
        }
    #endif // COMMENT

        if (col == 1)
        {
            return QString::fromStdString(barcodes[row]); 
        }
    }

    return QVariant();
}

Qt::ItemFlags CollectionModel::flags(const QModelIndex& index) const
{
    if (index.column() == 1)
        return Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    else
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant CollectionModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return "Index";
        case 1:
            return "Barcode";
        case 2:
            return "Current [nA]";
        case 3:
            return "Voltage [V]";
        case 4:
            return QVariant();
        }
    }

    return QVariant();
}

int CollectionModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return parameters.tubesPerChannel;
}

bool CollectionModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid())
        return false;
    
    int col = index.column();
    int row = index.row();

    if (col == 1)
    {
        barcodes[row] = value.toString().toStdString();
        return true;
    }

    return false;
}

void CollectionModel::receiveChannelPolarity(int polarity)
{
    this->polarity = polarity;
}

void CollectionModel::receiveTubeDataPacket(TubeData data)
{
    emit layoutAboutToBeChanged();

    if (data.channel == channel)
    {
        internalData[data.index] = data;
    }

    // QModelIndex topLeft = index(data.index, 2);
    // QModelIndex bottomRight = index(parameters.tubesPerChannel, 3);
    emit layoutChanged();
}

void CollectionModel::createFakeBarcodes()
{
    emit layoutAboutToBeChanged();

    for (int i = 0; i < barcodes.size(); ++i)
    {
        barcodes[i] = fmt::format("MSU0012{}", i);
    }

    emit layoutChanged();
}