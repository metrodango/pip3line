#include "packetsortfilterproxymodel.h"
#include "packet.h"
#include "shared/guiconst.h"
#include <QDebug>
#include <QHostAddress>

PacketSortFilterProxyModel::PacketSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    equalityViewEnabled = false;
    selectedPacket = PacketModelAbstract::INVALID_POS;
    packetModel = nullptr;
    setDynamicSortFilter(false);
}

PacketSortFilterProxyModel::~PacketSortFilterProxyModel()
{
    qDebug() << "Destroying" << this;
}

QVariant PacketSortFilterProxyModel::data(const QModelIndex &index, int role) const
{

    if (!index.isValid()) {
        return QVariant();
    }

    if (equalityViewEnabled &&
            role == Qt::BackgroundRole &&
            selectedPacket > PacketModelAbstract::INVALID_POS) {
        foreach (const int &col, equalityColumns) {
            QVariant selected = packetModel->data(packetModel->createIndex((int)selectedPacket, col),Qt::DisplayRole);
            QVariant current = packetModel->data(packetModel->createIndex(mapToSource(index).row(),col), Qt::DisplayRole);
            if (selected == current) {
                return GuiStyles::EqualPackets;
            }
        }
    }

    return QSortFilterProxyModel::data(index,role);
}

void PacketSortFilterProxyModel::setPacketSourceModel(PacketModelAbstract *originalModel)
{
    packetModel = originalModel;
    connect(packetModel, SIGNAL(updated()), SLOT(onModelUpdated()));
    QSortFilterProxyModel::setSourceModel(originalModel);
}
qint64 PacketSortFilterProxyModel::getSelectedPacket() const
{
    return selectedPacket;
}

void PacketSortFilterProxyModel::setSelectedPacket(const qint64 &packetNumber)
{
    if (packetNumber >= 0) {
        selectedPacket = packetNumber;
        emitGlobalDataChanged();
    } else {
        qCritical() << "[PacketSortFilterProxyModel::setSelectedPacket] invalid packet number:" << packetNumber;
    }
}

bool PacketSortFilterProxyModel::getEnableEqualityView() const
{
    return equalityViewEnabled;
}

void PacketSortFilterProxyModel::setEnableEqualityView(bool value)
{
    equalityViewEnabled = value;
}

void PacketSortFilterProxyModel::setEqualitycolumn(int column, bool enable)
{
    if (packetModel != nullptr && column < packetModel->columnCount()) {
        if (enable) {
            equalityColumns.insert(column);
        } else {
            equalityColumns.remove(column);
        }
        emitGlobalDataChanged();
    } else {
        qCritical() << tr("[PacketSortFilterProxyModel::addEqualitycolumn] Column does not exist, or model is nullptr T_T");
    }
}

void PacketSortFilterProxyModel::clearEqualityColumns()
{
    equalityColumns.clear();
    emitGlobalDataChanged();
}

bool PacketSortFilterProxyModel::isColumnEqualityenabled(int column) const
{
    return equalityColumns.contains(column);
}

void PacketSortFilterProxyModel::setColumnSortingType(int column, PacketSortFilterProxyModel::SortingTypes type)
{
    columnSorting.insert(column, type);
}

void PacketSortFilterProxyModel::onModelUpdated()
{
    sort(sortColumn());
}

//bool PacketSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
//{
//    int col1 = left.column();
//    int col2 = right.column();

//    if (col1 != col2) {
//        qCritical() << tr("[PacketSortFilterProxyModel::lessThan] Columns of the operands are not the same T_T");
//        return false;
//    }

//    if (columnSorting.contains(col1)) { // if there is a special sorting algorithm set
//        switch (columnSorting.value(col1)) {
//            case IP:
//            {
//                QHostAddress la;
//                QHostAddress ra;

//                if (la.setAddress(left.data().toString())) {
//                    if (ra.setAddress(right.data().toString())) {
//                        QAbstractSocket::NetworkLayerProtocol lp = la.protocol();
//                        QAbstractSocket::NetworkLayerProtocol rp = ra.protocol();
//                        if (lp == rp) { // if both are the same protocol
//                            if (lp == QAbstractSocket::IPv4Protocol) {
//                                return la.toIPv4Address() < ra.toIPv4Address();
//                            } else if (lp == QAbstractSocket::IPv6Protocol){
//                                Q_IPV6ADDR laddr = la.toIPv6Address();
//                                Q_IPV6ADDR raddr = ra.toIPv6Address();
//                                for (int i = 0; i < 16; ++i) {
//                                    if (laddr[i] == raddr[i])
//                                        continue;
//                                    else
//                                        return laddr[i] < raddr[i];
//                                }
//                                return false; // left is not less than right (i.e equal)
//                            }

//                        } else { // if the two protocols are differents
//                            return lp < rp; // following Qt definition "Unknown < IPv4 < IPv6"
//                        }
//                    } else { // if the right address is invalid
//                        return false; // again 'Unknown < IPv4 < IPv6'
//                    }
//                } else { // the left address is invalid
//                    if (ra.setAddress(right.data().toString())) { // but the right one is valid
//                        return true; // and again 'Unknown < IPv4 < IPv6'
//                    } else { // well, at this point there is no hope left, calling the default behaviour
//                        return QSortFilterProxyModel::lessThan(left,right);
//                    }
//                }
//            }
//                break;
//            default:
//                return QSortFilterProxyModel::lessThan(left,right);
//        }
//    }

//    // by default use the one form the parent
//    return QSortFilterProxyModel::lessThan(left,right);
//}

void PacketSortFilterProxyModel::emitGlobalDataChanged()
{
    if (packetModel != nullptr &&  packetModel->rowCount() > 0 && packetModel->columnCount() > 0) // no point of doing this is the data set is empty
        emit dataChanged(packetModel->createIndex(0,0), packetModel->createIndex(packetModel->rowCount() - 1, packetModel->columnCount() - 1));
}



