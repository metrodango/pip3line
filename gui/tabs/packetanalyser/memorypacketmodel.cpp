#include "memorypacketmodel.h"
#include "packet.h"
#include <threadedprocessor.h>
#include <transformmgmt.h>
#include "sources/blocksources/blockssource.h"
#include <QDebug>

MemoryPacketModel::MemoryPacketModel(TransformMgmt *transformFactory, QObject *parent) :
    PacketModelAbstract(transformFactory, parent)
{

}

MemoryPacketModel::~MemoryPacketModel()
{
    while (!packetsList.isEmpty())
        delete packetsList.takeFirst();
}

qint64 MemoryPacketModel::addPacket(Packet *packet)
{
    quint64 ret = PacketModelAbstract::INVALID_POS;
    if (packet != nullptr) {
        if (autoMergeConsecutivePackets && packetsList.size() > 0) {
            Packet * last = packetsList.last();
            if (last->getDirection() == packet->getDirection() && last->getDirection() != Packet::NODIRECTION) {
                QByteArray data = last->getOriginalData();
                data.append(packet->getOriginalData());
                last->setData(data,true);
                emit updated();
                return packetsList.size() - 1;
            }
        }

        int index = packetsList.size();
        beginInsertRows(QModelIndex(), index, index);
        ret = packetsList.size();
        packetsList.append(packet);
        endInsertRows();

        QHash<QString, QString> f = packet->getAdditionalFields();
        QHashIterator<QString, QString> i(f);
        while (i.hasNext()) {
            i.next();
            if (!columnNames.contains(i.key())) {
                addUserColumn(i.key(),nullptr,Pip3lineConst::TEXTFORMAT);
            }
        }

        emit updated();
    }

    return ret;
}

qint64 MemoryPacketModel::addPackets(QList<Packet *> packets)
{
    quint64 ret = PacketModelAbstract::INVALID_POS;
    if (packets.size() > 0) {
        int first = packetsList.size();
        int last = first < INT_MAX - packets.size() ? first + packets.size() : INT_MAX;
        int count = last - first;
        beginInsertRows(QModelIndex(), first, last);
        for (int i = 0 ; i < count ; i++) {
            packetsList.append(packets.at(i));
            QHash<QString, QString> f = packets.at(i)->getAdditionalFields();
            QHashIterator<QString, QString> j(f);
             while (j.hasNext()) {
                 j.next();
                 if (!columnNames.contains(j.key())) {
                     addUserColumn(j.key(),nullptr,Pip3lineConst::TEXTFORMAT);
                 }
             }
        }
        endInsertRows();

        ret = packetsList.size() > 0 ? packetsList.size() - 1 : PacketModelAbstract::INVALID_POS;
        emit updated();
    }

    return ret;
}

Packet *MemoryPacketModel::getPacket(qint64 index)
{
    if (index < (qint64)packetsList.size() && index >= 0) {
        return packetsList.at((int)index);
    } else {
        qCritical() << tr("[MemoryPacketModel::getPacket] index out-of-bound");
    }

    return nullptr;
}

void MemoryPacketModel::removePacket(qint64 index)
{
    if (index < (qint64)packetsList.size() && index >= 0) {
        beginRemoveRows(QModelIndex(), (int)index, (int)index);
        Packet * packet = packetsList.takeAt((int)index);
        delete packet;
        endRemoveRows();
        emit updated();
    } else {
        qCritical() << tr("[MemoryPacketModel::removePacket] index out-of-bound");
    }
}

void MemoryPacketModel::removePackets(QList<qint64> indexes)
{
    QList<Packet *> toBeDeleted;
    qint64 first = packetsList.size();
    qint64 last = 0;
    // get all the payloads to be deleted
    for (qint64 i = 0; i < (qint64)indexes.size(); i++) {
        qint64 index = indexes.at(i);
        if (index >= 0 && index < (qint64)packetsList.size()) {
            first = qMin(first,index);
            last = qMax(last,index);
            toBeDeleted.append(packetsList.at((int)index));
        }
    }

    if (first <= last) { // if this is not true, trouble!!
        // delete them
        beginRemoveRows(QModelIndex(), (int)first, (int)last);
        while (!toBeDeleted.isEmpty()) {
            Packet *dt = toBeDeleted.takeFirst();
            int res = packetsList.removeAll(dt); // should not have duplicate problems -_-'
            if (res > 1)
                qCritical() << tr("more than one payload was removed T_T");
            delete dt;
        }
        endRemoveRows();
        emit updated();
    } else {
        qCritical() << tr("[MemoryPacketModel::removePackets] first > last T_T");
    }
}

qint64 MemoryPacketModel::merge(QList<qint64> list)
{
    qint64 pindex = PacketModelAbstract::INVALID_POS;
    if (list.size() > 1) { // only merge if there is more than two selected
        QByteArray temp;
        QList<Packet *> toBeDeleted;

        // take the first one
        pindex = list.at(0);
        qint64 first = pindex;
        qint64 last = first;

        Packet * newp = nullptr;
        if (first > -1 && first < packetsList.size()) {
            newp = packetsList.at((int)first);
            temp.append(newp->getData());
        }
        else {
            qCritical() << tr("first payload index is not there T_T");
            return INVALID_POS;
        }

        // then create the resulting payload
        for (qint64 i = 1; i < list.size(); i++) {
            qint64 index = list.at(i);
            if (index > -1 && index < packetsList.size()) {
                Packet * p = packetsList.at((int)index);
                first = qMin(first,index);
                last = qMax(last,index);
                toBeDeleted.append(p);
                temp.append(p->getData());
            } else {
                qCritical() << tr("Payload index is not there T_T");
                return INVALID_POS;
            }
        }

        // set the first one to be the resulting payload, and remove the others

        newp->setOriginalData(temp);
        QModelIndex modified1;
        QModelIndex modified2;

        modified1 = QAbstractTableModel::createIndex(pindex, 0);
        modified2 = QAbstractTableModel::createIndex(pindex, columnNames.size() - 1);
        emit dataChanged(modified1, modified2);

        if (first <= last) { // if this is not true, trouble!!
            if (toBeDeleted.size() == packetsList.size()) {
                first = 1;
                pindex = 0;
            }

            // at this point first and last should be < MAX_INT
            beginRemoveRows(QModelIndex(),(int)first + 1, (int)last ); // the first one is not going to be deleted
            qDebug() << toBeDeleted.size() << "rows to be removed. Initial list" << packetsList.size();
            for (int i = 0; i < toBeDeleted.size(); i++) {
                int res = packetsList.removeAll(toBeDeleted.at(i)); // should not have duplicate problems -_-'
                if (res > 1)
                    qCritical() << tr("more than one payload was remvoved T_T");
                delete toBeDeleted.at(i);
            }
            endRemoveRows();
            emit updated();
        } else {
            qCritical() << tr("first and last are wrong T_T");
        }

    }

    return pindex; // returning the resulting index
}

qint64 MemoryPacketModel::size() const
{
    return packetsList.size();
}

void MemoryPacketModel::clear()
{
    beginResetModel();
    while (!packetsList.isEmpty())
        delete packetsList.takeFirst();
    endResetModel();
    emit updated();
}

void MemoryPacketModel::transformRequestFinished(QList<QByteArray> dataList, Messages messages)
{
    quintptr senderID = (quintptr)sender();
    if (transformRequests.contains(senderID)) {
        QPair<int,int> target = transformRequests.take(senderID);
        if (target.second > lastPredefinedColumn && target.second < columnNames.size()) {

            int size = dataList.size();
            if (size > packetsList.size() - target.first) {
                qWarning() << tr("[MemoryPacketModel::transformRequestFinished] data set size (%1) is too big")
                               .arg(dataList.size());
                size = packetsList.size() - target.first; // reducing the saice
            }

            QString colName = columnNames.at(target.second);
            OutputFormat format = userColumnsDef.value(colName).format;
            int j = 0;
            for (int i = target.first; i < target.first + size; i++) {
                Packet * pa = packetsList.at(i);
                QString data = QString::fromUtf8((format == Pip3lineConst::TEXTFORMAT) ? dataList.at(j) : dataList.at(j).toHex());
                pa->setAdditionalField(colName, data);
                j++;
            }
            emit dataChanged(createIndex(target.first, target.second), createIndex(target.first + size - 1, target.second));

            // loggin error messages

            for (int i = 0; i < messages.size(); i++) {
                emit log(messages.at(i).message,messages.at(i).source, messages.at(i).level);
            }

        } else {
            qCritical() << tr("[MemoryPacketModel::transformRequestFinished] Can't find the column %1 T_T").arg(target.second);
        }
    } else {
        qCritical() << tr("[MemoryPacketModel::transformRequestFinished] Can't find the request id T_T");
    }
    emit readOnlyStateChanged(false);
}

void MemoryPacketModel::transformUpdated()
{
    TransformAbstract * transform = dynamic_cast<TransformAbstract *>(sender());
    if (transform != nullptr) {
        QString colName;
        //retrieving the column name
        QHashIterator<QString, Usercolumn> i(userColumnsDef);
         while (i.hasNext()) {
             i.next();
             if (i.value().transform == transform) {
                 colName = i.key();
                 break;
             }
         }

         if (colName.isEmpty()) {
             qCritical() << tr("[MemoryPacketModel::transformUpdated] cannot find the column name based on the Transform object T_T");
             return;
         }

         int column = getColumnIndex(colName);
         if (column == -1) {
             return;
         }
         launchUpdate(transform, 0,column);

    } else {
        qCritical() << tr("[MemoryPacketModel::transformUpdated] dynamic cast failed T_T");
    }
}

void MemoryPacketModel::refreshColumn(const QString colName)
{
    if (userColumnsDef.contains(colName) && packetsList.size() > 0) {
        TransformAbstract * transform = userColumnsDef.value(colName).transform;
        int column = getColumnIndex(colName);
        if (column == -1) {
            return;
        }
        launchUpdate(transform, 0,column);
    }
}

void MemoryPacketModel::internalAddUserColumn(const QString &name, TransformAbstract *transform)
{
    int column = getColumnIndex(name);
    if (column == -1) {
        return;
    }

    launchUpdate(transform, 0,column);
}

QVariant MemoryPacketModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();
    const Packet * packet = packetsList.value(row,nullptr);

    return payloadData(packet,column, role);
}

int MemoryPacketModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return packetsList.size();
}

void MemoryPacketModel::launchUpdate(TransformAbstract *transform, int row, int column, int length)
{
//   qDebug() << "update row" << row << "column" << column << "length" << length;
    if (transform == nullptr)
        return; // ntoghin to do here

    if (row < 0 || row >= packetsList.size()) {
        qCritical() << tr("[MemoryPacketModel::launchUpdate] Row value out-of-bound T_T");
        return;
    }
    if (length < 0) { // all to the end of the list
        length = packetsList.size() - row;
    }

    if (row > packetsList.size() - length) {
        qCritical() << tr("[MemoryPacketModel::launchUpdate] Row + length value is out-of-bound T_T");
        return;
    }

    TransformAbstract * ta = transformFactory->cloneTransform(transform);
    if (ta != nullptr) {
        QList<QByteArray> dataList;
        for (int i = row; i < row + length; i++) {
            dataList.append(packetsList.at(i)->getData());
        }

        TransformRequest *tr = new(std::nothrow) TransformRequest(
                    ta,
                    dataList,
                    0);

        if (tr == nullptr) {
            qFatal("Cannot allocate memory for TransformRequest X{");
        }

        connect(tr,SIGNAL(finishedProcessing(QList<QByteArray>,Messages)), this, SLOT(transformRequestFinished(QList<QByteArray>,Messages)));
        QPair<int,int> coordinate(row, column);
        transformRequests.insert((quintptr)tr, coordinate);
        emit readOnlyStateChanged(true);
        emit sendRequest(tr);
    }
}

void MemoryPacketModel::clearAdditionalField(const QString &name)
{
    beginResetModel();
    for (int i = 0; i < packetsList.size(); i++) {
        packetsList.at(i)->removeAdditionalFields(name);
    }
    endResetModel();
}

QItemSelection MemoryPacketModel::getLastPacketRow()
{
    QItemSelection selection;
    if (packetsList.size() > 0)
        selection.select(createIndex(packetsList.size() - 1, 0), createIndex(packetsList.size() - 1, columnNames.size() - 1));

    return selection;
}

void MemoryPacketModel::mergeConsecutivePackets()
{
    if (packetsList.size() > 1) { // no need to merge anything if size() < 2
        beginResetModel();
        Packet * previous = packetsList.at(0); // take the first packet
        QByteArray data = previous->getData();
        for (int i = 1; i < packetsList.size(); i++) {
            Packet * current = packetsList.at(i);
            if (previous->getDirection() == current->getDirection() &&  // if they have the same direction
                   previous->getDirection() != Packet::NODIRECTION &&   // and if this direction is an actual one
                   previous->getSourceid() == current->getSourceid() && // and if they have the same source id
                   previous->getSourceid() != Block::INVALID_ID ) {     // and if this source ID is valid
                data.append(current->getData());
                delete packetsList.takeAt(i);
                i--; // removed a packet so need to reduce i
            } else {
                previous->setOriginalData(data);
                data = current->getData();
                previous = current;
            }
        }

        previous->setOriginalData(data);

        endResetModel();

        emit updated();
    }
}

