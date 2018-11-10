#ifndef PACKETSORTFILTERPROXYMODEL_H
#define PACKETSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QSet>
#include <QHash>
#include "filteritem.h"
#include "filterengine.h"
#include "packetmodelabstract.h"
#include "state/basestateabstract.h"

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
        ~PacketSortFilterProxyModel() override;

        QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
        void setPacketSourceModel ( PacketModelAbstract * originalModel );

        qint64 getSelectedPacket() const;
        void setSelectedPacket(const qint64 &packetNumber);

        qint64 indexToPacketIndex(const QModelIndex & index);

        bool getEnableEqualityView() const;
        void setEnableEqualityView(bool value);
        void setEqualitycolumn(int column, bool enable = true);
        void clearEqualityColumns();
        bool isColumnEqualityenabled(int column) const;

        void setColumnSortingType(int column, SortingTypes type);

        QHash<QString, QString> getConfiguration();
        void setConfiguration(const QHash<QString, QString> &conf);
        BaseStateAbstract *getStateMngtObj();
        FilterEngine *getFilterEngine() const;
        void setFilterEngine(FilterEngine *value);

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    private slots:
        void onModelUpdated();
        void onFitlersUpdated();
    private:
        Q_DISABLE_COPY(PacketSortFilterProxyModel)
        //        bool lessThan(const QModelIndex & left, const QModelIndex & right) const;
        void emitGlobalDataChanged();
        QSet<int> equalityColumns;
        QHash<int, SortingTypes> columnSorting;
        bool equalityViewEnabled;
        qint64 selectedPacket;
        PacketModelAbstract * packetModel;
        FilterEngine *filterEngine;

        friend class SortFilterProxyStateObj;
};

class SortFilterProxyStateObj : public BaseStateAbstract
{
        Q_OBJECT
    public:
        explicit SortFilterProxyStateObj(PacketSortFilterProxyModel *proxyModel);
        virtual ~SortFilterProxyStateObj();
        virtual void run();
    protected:
        PacketSortFilterProxyModel *proxyModel;
    private:
        Q_DISABLE_COPY(SortFilterProxyStateObj)
};

#endif // PACKETSORTFILTERPROXYMODEL_H
