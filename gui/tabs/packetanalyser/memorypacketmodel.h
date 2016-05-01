/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MEMORYPACKETMODEL_H
#define MEMORYPACKETMODEL_H

#include "packetmodelabstract.h"
#include <QStringList>

class Packet;

class MemoryPacketModel : public PacketModelAbstract
{
    public:
        explicit MemoryPacketModel(TransformMgmt *transformFactory, QObject *parent = nullptr);
        ~MemoryPacketModel();

        Packet *getPacket(qint64 index);
        void removePacket(qint64 index);
        void removePackets(QList<qint64> indexes);
        qint64 merge(QList<qint64> list);
        qint64 size() const;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

        void clearAdditionalField(const QString &name);
        QItemSelection getLastPacketRow();
    public slots:
        void mergeConsecutivePackets();
        qint64 addPacket(Packet * packet);
        qint64 addPackets(QList<Packet *> packets);
        void clear();
        void transformRequestFinished(QList<QByteArray> dataList, Messages messages);
        void transformUpdated();
        void refreshColumn(const QString colName);
    private:
        void launchUpdate(TransformAbstract * transform, int row, int column,int length = -1);
        void internalAddUserColumn(const QString &name, TransformAbstract * transform);
        QList<Packet *> packetsList;
};

#endif // MEMORYPACKETMODEL_H
