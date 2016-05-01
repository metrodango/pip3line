#ifndef PACKETSORTFILTERPROXYMODEL_H
#define PACKETSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QSet>
#include "packetmodelabstract.h"

class PacketSortFilterProxyModel : public QSortFilterProxyModel
{
        Q_OBJECT
    public:
        enum SortingTypes {
            NONE = -1,
            ALPHABETICAL = 0,
            IP = 1,
            NUMBER = 2
        };

        explicit PacketSortFilterProxyModel(QObject *parent = nullptr);
        ~PacketSortFilterProxyModel();

        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
        void setPacketSourceModel ( PacketModelAbstract * originalModel );

        qint64 getSelectedPacket() const;
        void setSelectedPacket(const qint64 &packetNumber);

        bool getEnableEqualityView() const;
        void setEnableEqualityView(bool value);

        void setEqualitycolumn(int column, bool enable = true);
        void clearEqualityColumns();
        bool isColumnEqualityenabled(int column) const;

        void setColumnSortingType(int column, SortingTypes type);
    private slots:
        void onModelUpdated();

    private:
//        bool lessThan(const QModelIndex & left, const QModelIndex & right) const;
        void emitGlobalDataChanged();
        QSet<int> equalityColumns;
        QHash<int, SortingTypes> columnSorting;
        bool equalityViewEnabled;
        qint64 selectedPacket;
        PacketModelAbstract * packetModel;
};

#endif // PACKETSORTFILTERPROXYMODEL_H
