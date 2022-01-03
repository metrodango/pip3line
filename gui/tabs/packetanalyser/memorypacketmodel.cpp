#include "memorypacketmodel.h"
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
    packetsList.clear(); // shared pointer are freed here
}

qint64 MemoryPacketModel::addPacket(const QSharedPointer<Packet> & packet)
{
    qint64 ret = PacketModelAbstract::INVALID_POS;
    if (packet != nullptr) {
        if (autoMergeConsecutivePackets && packetsList.size() > 0) {
            QSharedPointer<Packet> last = packetsList.last();
            if (arePacketsMergeable(last,packet)) {
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

qint64 MemoryPacketModel::addPackets(const QList<QSharedPointer<Packet> > &packets)
{
    qint64 ret = PacketModelAbstract::INVALID_POS;
    if (packets.size() > 0) {
        int first = packetsList.size();
        int last = first < INT_MAX - packets.size() ? first + packets.size() : INT_MAX;
        int count = last - first;
        if (count > 0) {
            beginInsertRows(QModelIndex(), first, last - 1);
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
    }



    return ret;
}

QSharedPointer<Packet> MemoryPacketModel::getPacket(qint64 index)
{
    if (index < static_cast<qint64>(packetsList.size()) && index >= 0) {
        return packetsList.at(static_cast<int>(index));
    } else {
        qCritical() << tr("[MemoryPacketModel::getPacket] index out-of-bound");
    }

    return QSharedPointer<Packet>();
}

QSharedPointer<Packet> MemoryPacketModel::getPreviousPacket(qint64 index, Packet::Direction direction)
{
    QSharedPointer<Packet> ret = QSharedPointer<Packet>();
    if (index > 0 // there is not packet before the first one
            && index < static_cast<qint64>(packetsList.size())) {
        index--;
        if (direction == Packet::NODIRECTION) { // just the previous one then
            ret = packetsList.at(static_cast<int>(index));
        } else {
            while (index >=0) {
                QSharedPointer<Packet> current = packetsList.at(static_cast<int>(index));
                if (current->getDirection() == direction) {
                    ret = current;
                    break;
                }
                index--;
            }
        }
    }
    return ret;
}

void MemoryPacketModel::removePacket(qint64 index)
{
    if (index < static_cast<qint64>(packetsList.size()) && index >= 0) {
        beginRemoveRows(QModelIndex(), static_cast<int>(index), static_cast<int>(index));
        QSharedPointer<Packet> packet = packetsList.takeAt(static_cast<int>(index));
        packet.clear();
        endRemoveRows();
        emit updated();
    } else {
        qCritical() << tr("[MemoryPacketModel::removePacket] index out-of-bound");
    }
}

void MemoryPacketModel::removePackets(QList<qint64> &indexes)
{
    QList<QSharedPointer<Packet> > toBeDeleted;

    // sorting packets by index
    std::sort(indexes.begin(),indexes.end(), std::less<qint64>());

    // get all the payloads to be deleted
    for (int i = 0; i < indexes.size(); i++) {
        qint64 index = indexes.at(i);
        if (index >= 0 && index < static_cast<qint64>(packetsList.size())) {
            toBeDeleted.append(packetsList.at(static_cast<int>(index)));
        }
    }

    int lindex = 0;

    while (!toBeDeleted.isEmpty()) {
        QSharedPointer<Packet> dt = toBeDeleted.takeFirst();
        lindex = packetsList.indexOf(dt, lindex); // values are ordered so we can start the search beginning at the last occurence
        beginRemoveRows(QModelIndex(), lindex, lindex);
        int res = packetsList.removeAll(dt); // should not have duplicate problems -_-'
        endRemoveRows();
        if (res > 1)
            qCritical() << tr("more than one payload was removed T_T");
        dt.clear();
    }

    emit updated();
}

qint64 MemoryPacketModel::merge(QList<qint64> &list)
{
    qint64 pindex = PacketModelAbstract::INVALID_POS;
    if (list.size() > 1) { // only merge if there is more than two selected
        QByteArray temp;
        QList<QSharedPointer<Packet> > toBeDeleted;

        // sorting packets by index
        std::sort(list.begin(),list.end(), std::less<qint64>());

        // take the first one
        pindex = list.at(0);

        QSharedPointer<Packet> newp;
        if (pindex > -1 && pindex < packetsList.size()) {
            newp = packetsList.at(static_cast<int>(pindex));
            temp.append(newp->getData());
        }
        else {
            qCritical() << tr("[MemoryPacketModel::merge] first payload index is not there T_T");
            return INVALID_POS;
        }

        // then create the resulting payload
        // get the first packet
        qint64 first = list.at(1); // list.size > 1
        // get the last
        qint64 last = list.last();
        for (int i = 1; i < list.size(); i++) {
            qint64 index = list.at(i);
            if (index > -1 && index < packetsList.size()) {
                QSharedPointer<Packet> p = packetsList.at(static_cast<int>(index));

                toBeDeleted.append(p);
                temp.append(p->getData());
            } else {
                qCritical() << tr("[MemoryPacketModel::merge] Selected payload index (%1) is invalid T_T, ignoring.. ").arg(i);
            }
        }

        // set the first one to be the resulting payload, and remove the others

        newp->setOriginalData(temp);

        if (first <= last) { // if this is not true, trouble!!
            if (toBeDeleted.size() == packetsList.size()) {
                qCritical() << tr("[MemoryPacketModel::merge] toBeDeleted size cannot be equal to packet list size ...");
                first = 1;
                pindex = 0;
            }
            int lindex = 0;
            qDebug() << toBeDeleted.size() << "rows to be removed. Initial list" << packetsList.size();
            for (int i = 0; i < toBeDeleted.size(); i++) {
                QSharedPointer<Packet> p  = toBeDeleted.at(i);
                lindex = packetsList.indexOf(p, lindex); // values are ordered so no we can start the search at the last
                beginRemoveRows(QModelIndex(),lindex, lindex );
                int res = packetsList.removeAll(p); // should not have duplicate problems -_-'
                if (res > 1)
                    qCritical() << tr("[MemoryPacketModel::merge] More than one payload was remvoved T_T");
                p.clear();
                endRemoveRows();
            }

            emit updated();
        } else {
            qCritical() << tr("[MemoryPacketModel::merge] first and last are wrong T_T");
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
    packetsList.clear();
    endResetModel();
    emit updated();
}

void MemoryPacketModel::transformRequestFinished(QList<QByteArray> dataList, Messages messages)
{
    quintptr senderID = reinterpret_cast<quintptr>(sender());
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
                QSharedPointer<Packet> pa = packetsList.at(i);
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

bool MemoryPacketModel::arePacketsMergeable(QSharedPointer<Packet> pone, QSharedPointer<Packet> ptwo)
{
    return (pone->getDirection() == ptwo->getDirection() &&  // if they have the same direction
            pone->getDirection() != Packet::NODIRECTION &&   // and if this direction is an actual one
            pone->getSourceid() == ptwo->getSourceid() &&    // and if they have the same source id
            pone->getSourceid() != Block::INVALID_ID ) ;     // and if this source ID is valid
}

QVariant MemoryPacketModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int row = index.row();
    int column = index.column();
    const QSharedPointer<Packet> packet = packetsList.value(row,QSharedPointer<Packet>());

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
    if (transform == nullptr || packetsList.size() == 0)
        return; // nothing to do here

    if (row < 0 || row >= packetsList.size()) {
        qCritical() << tr("[MemoryPacketModel::launchUpdate] Row value out-of-bound: %1 T_T").arg(row);
        return;
    }
    if (length < 0) { // all to the end of the list
        length = packetsList.size() - row;
    }

    if (row > packetsList.size() - length) {
        qCritical() << tr("[MemoryPacketModel::launchUpdate] Row (%1) + length (%2) value is out-of-bound T_T").arg(row).arg(length);
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

        //connect(tr, qOverload<QList<QByteArray>,Messages >(&TransformRequest::finishedProcessing), this, &MemoryPacketModel::transformRequestFinished);
        connect(tr, SIGNAL(finishedProcessing(QList<QByteArray>,Messages)), this, SLOT(transformRequestFinished(QList<QByteArray>,Messages)));
        QPair<int,int> coordinate(row, column);
        transformRequests.insert(reinterpret_cast<quintptr>(tr), coordinate);
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
        QSharedPointer<Packet> previous = packetsList.at(0); // take the first packet
        QByteArray data = previous->getData();
        for (int i = 1; i < packetsList.size(); i++) {
            QSharedPointer<Packet> current = packetsList.at(i);
            if (arePacketsMergeable(previous,current)) {  // checking mergeability
                data.append(current->getData());
                packetsList.takeAt(i).clear();
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

