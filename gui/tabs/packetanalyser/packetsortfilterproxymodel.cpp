#include "packetsortfilterproxymodel.h"
#include "packet.h"
#include "shared/guiconst.h"
#include <QDebug>
#include <QHostAddress>
#include <QPalette>
#include <QSetIterator>
#include <QStringList>

PacketSortFilterProxyModel::PacketSortFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
    equalityViewEnabled = false;
    selectedPacket = PacketModelAbstract::INVALID_POS;
    packetModel = nullptr;
    filteringEnabled = false;
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
        (role == Qt::BackgroundRole || role == Qt::ForegroundRole) &&
        selectedPacket > PacketModelAbstract::INVALID_POS) {

        bool areEquals = false;
        foreach (const int &col, equalityColumns) {
           // qDebug() << tr("mapToSource usage");
            Packet * selected = packetModel->getPacket(selectedPacket);
            if (selected != nullptr) {
                QModelIndex ori = mapToSource(index);
                if (ori.isValid()) {
                    Packet * current = packetModel->getPacket(ori.row());
                    if (col == PacketModelAbstract::COLUMN_PAYLOAD) {
                        areEquals = (selected->getData() == current->getData());
                        if (!areEquals)
                            break;
                    } else if (packetModel->isUserColumn(col)) {
                        QString colName = packetModel->getColumnName(col);
                        areEquals = (selected->fetchAdditionalField(colName) == current->fetchAdditionalField(colName));
                        if (!areEquals)
                            break;
                    } else if (col == PacketModelAbstract::COLUMN_DIRECTION){
                        areEquals = (selected->getDirection() == current->getDirection());
                        if (!areEquals)
                            break;
                    } else if (col == PacketModelAbstract::COLUMN_LENGTH){
                        areEquals = (selected->getData().size() == current->getData().size());
                        if (!areEquals)
                            break;
                    } else if (col == PacketModelAbstract::COLUMN_CID){
                        areEquals = (selected->getSourceid() == current->getSourceid());
                        if (!areEquals)
                            break;
                    } else if (col == PacketModelAbstract::COLUMN_TIMESPTAMP){
                        areEquals = (selected->getTimestamp() == current->getTimestamp());
                        if (!areEquals)
                            break;
                    } else {
                        QVariant selectedv = packetModel->data(packetModel->createIndex((int)selectedPacket, col),Qt::DisplayRole);
                        QVariant currentv = packetModel->data(packetModel->createIndex(ori.row(),col), Qt::DisplayRole);
                        areEquals = (selectedv.isValid() && currentv.isValid() && selectedv == currentv);
                        if (!areEquals)
                            break;
                    }

                } else {
                    qCritical() << tr("[PacketSortFilterProxyModel::data] Invalid mapping");
                }
            }
        }

        if (areEquals) {
            if (role == Qt::BackgroundRole)
                return GlobalsValues::EqualsPacketsBackground;
            else
                return GlobalsValues::EqualsPacketsForeground;
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

bool PacketSortFilterProxyModel::isFilteringEnabled() const
{
    return filteringEnabled;
}

void PacketSortFilterProxyModel::setFilteringEnabled(bool value)
{
    beginResetModel();
    filteringEnabled = value;
    endResetModel();
}

QHash<QString, QString> PacketSortFilterProxyModel::getConfiguration()
{
    QHash<QString, QString> ret;

    ret.insert(GuiConst::STATE_PACKET_PROXY_FILTERS_ENABLE, filteringEnabled ? QString("1") : QString("0"));
    ret.insert(GuiConst::STATE_PACKET_PROXY_EQUALITY_ENABLE, equalityViewEnabled ? QString("1") : QString("0"));
    QSetIterator<int> i(equalityColumns);
    QString equalSTR;
    while (i.hasNext())
        equalSTR.append(QString::number(i.next()))
                .append(GuiConst::STATE_FIELD_SEPARATOR);
    if (!equalSTR.isEmpty()) {
        ret.insert(GuiConst::STATE_PACKET_PROXY_EQUALITY_TARGET_COLUMNS, equalSTR);
    }

    ret.insert(GuiConst::STATE_PACKET_PROXY_SORTING_TARGET_COLUMN, QString::number(sortColumn()));
    ret.insert(GuiConst::STATE_PACKET_PROXY_SORTING_ORDER, QString::number((int)sortOrder()));

    return ret;
}

void PacketSortFilterProxyModel::setConfiguration(const QHash<QString, QString> &conf)
{
    bool ok = false;
    if (conf.contains(GuiConst::STATE_PACKET_PROXY_FILTERS_ENABLE)) {
        int val = conf.value(GuiConst::STATE_PACKET_PROXY_FILTERS_ENABLE).toInt(&ok);
        if (ok && (val == 0 || val == 1)) {
            filteringEnabled = (val == 1);
        }
    }

    if (conf.contains(GuiConst::STATE_PACKET_PROXY_EQUALITY_ENABLE)) {
        int val = conf.value(GuiConst::STATE_PACKET_PROXY_EQUALITY_ENABLE).toInt(&ok);
        if (ok && (val == 0 || val == 1)) {
            equalityViewEnabled = (val == 1);
        }
    }

    if (conf.contains(GuiConst::STATE_PACKET_PROXY_EQUALITY_TARGET_COLUMNS)) {
        QStringList equalList = conf.value(GuiConst::STATE_PACKET_PROXY_EQUALITY_TARGET_COLUMNS).split(GuiConst::STATE_FIELD_SEPARATOR, QString::SkipEmptyParts);
        if (!equalList.isEmpty()) {
            equalityColumns.clear();
            for (int i = 0; i < equalList.size(); i++) {
                int column = equalList.at(i).toInt(&ok);
                if (ok && packetModel != nullptr && column < packetModel->columnCount()) {
                        equalityColumns.insert(column);
                }
            }
        }
    }

    if (conf.contains(GuiConst::STATE_PACKET_PROXY_SORTING_TARGET_COLUMN)) {
        int val = conf.value(GuiConst::STATE_PACKET_PROXY_SORTING_TARGET_COLUMN).toInt(&ok);
        if (ok && val >= 0) {
            if (conf.contains(GuiConst::STATE_PACKET_PROXY_SORTING_ORDER)) {
                int order = conf.value(GuiConst::STATE_PACKET_PROXY_SORTING_ORDER).toInt(&ok);
                if (ok && (order == (int)Qt::AscendingOrder || order == (int)Qt::DescendingOrder)) {
                    sort(val, (Qt::SortOrder) order);
                }
            }
        }
    }
}

BaseStateAbstract *PacketSortFilterProxyModel::getStateMngtObj()
{
    SortFilterProxyStateObj *stateObj = new(std::nothrow) SortFilterProxyStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for SortFilterProxyStateObj X{");
    }
    return stateObj;
}

bool PacketSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & /*sourceParent*/) const
{
    bool ret = true;

    if (filteringEnabled) {
        QByteArray data = packetModel->getPacket(sourceRow)->getData();
        for (int i = 0; i < filterList.size(); i++) {
            FilterItem it = filterList.at(i);
            ret = it.selectable(data);
//            if (data.contains(it.getValue())) {
//                ret = true;
//            } else {
//                ret = false;
//            }
//            if (it.isReverseSelection()) {
//                ret = !ret;
//            }

            if (!ret) // stopping at this filter
                break;
        }
    }
    return ret;
}

QList<FilterItem> PacketSortFilterProxyModel::getFilterList() const
{
    return filterList;
}

void PacketSortFilterProxyModel::setFilterList(const QList<FilterItem> &value)
{
    beginResetModel();
    filterList = value;
    endResetModel();
}




SortFilterProxyStateObj::SortFilterProxyStateObj(PacketSortFilterProxyModel *proxyModel) :
    proxyModel(proxyModel)
{
    name = metaObject()->className();
}

SortFilterProxyStateObj::~SortFilterProxyStateObj()
{

}

void SortFilterProxyStateObj::run()
{
    QHash<QString, QString> conf;
    QList<FilterItem> filterList = proxyModel->filterList;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        // saving sorting/filtering information
        writer->writeStartElement(GuiConst::STATE_FILTER_SORT);
        conf = proxyModel->getConfiguration();
        QHashIterator<QString, QString> hi(conf);
        while (hi.hasNext()) {
            hi.next();
            writer->writeAttribute(hi.key(), hi.value());
        }

        writer->writeStartElement(GuiConst::STATE_FILTER_ITEMS);
        for (int i = 0; i < proxyModel->filterList.size(); i++) {
            writer->writeStartElement(GuiConst::STATE_FILTER_ITEM);
            FilterItem item = filterList.at(i);
            conf = item.getConfiguration();
            QHashIterator<QString, QString> it(conf);
            while (it.hasNext()) {
                it.next();
                writer->writeAttribute(it.key(), it.value());
            }
            writer->writeEndElement(); // STATE_FILTER_ITEM
        }
        writer->writeEndElement(); // STATE_FILTER_ITEMS
        writer->writeEndElement(); // STATE_FILTER_SORT
    } else {
        // restoring view tabs
        if (reader->name() == GuiConst::STATE_FILTER_SORT || readNextStart(GuiConst::STATE_FILTER_SORT)) {
            QXmlStreamAttributes attrList = reader->attributes();
            conf.clear();
            for (int i = 0; i < attrList.size(); i++) {
                QXmlStreamAttribute attr = attrList.at(i);
                conf.insert(attr.name().toString(), attr.value().toString());
            }

            proxyModel->setConfiguration(conf);

            if (readNextStart(GuiConst::STATE_FILTER_ITEMS)) {
                filterList.clear();
                reader->readNext();
                while (reader->name() == GuiConst::STATE_FILTER_ITEM) {
                    attrList = reader->attributes();
                    conf.clear();
                    for (int i = 0; i < attrList.size(); i++) {
                        QXmlStreamAttribute attr = attrList.at(i);
                        conf.insert(attr.name().toString(), attr.value().toString());
                    }
                    FilterItem item;
                    item.setConfiguration(conf);
                    if (item.isValid()) {
                        filterList.append(item);
                    }

                    proxyModel->setFilterList(filterList);
                    readEndElement(GuiConst::STATE_FILTER_ITEM);
                    reader->readNext();
                }
                if (reader->name() == GuiConst::STATE_FILTER_ITEMS && reader->tokenType() == QXmlStreamReader::StartElement )
                    readEndElement(GuiConst::STATE_FILTER_ITEMS);
            }
            readEndElement(GuiConst::STATE_FILTER_SORT);
        }
    }
}
