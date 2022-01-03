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
#include <pipelinecommon.h>
using namespace Pip3lineCommon;

class MemoryPacketModel : public PacketModelAbstract
{
        Q_OBJECT
    public:
        explicit MemoryPacketModel(TransformMgmt *transformFactory, QObject *parent = nullptr);
        ~MemoryPacketModel() override;

        QSharedPointer<Packet> getPacket(qint64 index) override;
        QSharedPointer<Packet> getPreviousPacket(qint64 index, Packet::Direction direction = Packet::NODIRECTION) override;
        void removePacket(qint64 index) override;
        void removePackets(QList<qint64> &indexes) override;
        qint64 merge(QList<qint64> &list) override;
        qint64 size() const override;

        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;

        void clearAdditionalField(const QString &name) override;
        QItemSelection getLastPacketRow() override;
    public slots:
        void mergeConsecutivePackets() override;
        qint64 addPacket(const QSharedPointer<Packet> &packet) override;
        qint64 addPackets(const QList<QSharedPointer<Packet> > &packets) override;
        void clear() override;
        void transformRequestFinished(QList<QByteArray> dataList, Messages messages) override;
        void transformUpdated() override;
        void refreshColumn(const QString colName) override;
    private:
        Q_DISABLE_COPY(MemoryPacketModel)
        void launchUpdate(TransformAbstract * transform, int row, int column,int length = -1) override;
        void internalAddUserColumn(const QString &name, TransformAbstract * transform) override;
        bool arePacketsMergeable(QSharedPointer<Packet>  pone, QSharedPointer<Packet> ptwo);
        QList<QSharedPointer<Packet> > packetsList;
};

#endif // MEMORYPACKETMODEL_H
