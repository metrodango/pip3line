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
    setDynamicSortFilter(false);
    filterEngine = new(std::nothrow)FilterEngine(this);
    if (filterEngine == nullptr) {
        qFatal("Cannot allocate FilterEngine X{");
    }

    connect(filterEngine, &FilterEngine::updated, this, &PacketSortFilterProxyModel::onFitlersUpdated);
}

PacketSortFilterProxyModel::~PacketSortFilterProxyModel()
{
    delete filterEngine;
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
            QSharedPointer<Packet> selected = packetModel->getPacket(selectedPacket);
            if (selected != nullptr) {
                QModelIndex ori = mapToSource(index);
                if (ori.isValid()) {
                    QSharedPointer<Packet> current = packetModel->getPacket(ori.row());
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
                        QVariant selectedv = packetModel->data(packetModel->createIndex(static_cast<int>(selectedPacket), col),Qt::DisplayRole);
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
    connect(packetModel, &PacketModelAbstract::updated, this, &PacketSortFilterProxyModel::onModelUpdated);
    QSortFilterProxyModel::setSourceModel(originalModel);
}
qint64 PacketSortFilterProxyModel::getSelectedPacket() const
{
    return selectedPacket;
}

void PacketSortFilterProxyModel::setSelectedPacket(const qint64 &packetNumber)
{
    if (packetNumber >= PacketModelAbstract::INVALID_POS && packetNumber < packetModel->size()) {
        selectedPacket = packetNumber;
    } else {
        qCritical() << "[PacketSortFilterProxyModel::setSelectedPacket] invalid packet number:" << packetNumber;
        selectedPacket = PacketModelAbstract::INVALID_POS;
    }

    emitGlobalDataChanged();
}

qint64 PacketSortFilterProxyModel::indexToPacketIndex(const QModelIndex &index)
{
    QModelIndex trueIndex = mapToSource(index);
    return packetModel->indexToPacketIndex(trueIndex);
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


void PacketSortFilterProxyModel::onFitlersUpdated()
{
    beginResetModel();
    endResetModel();
}

void PacketSortFilterProxyModel::emitGlobalDataChanged()
{
    if (packetModel != nullptr &&  packetModel->rowCount() > 0 && packetModel->columnCount() > 0) // no point of doing this is the data set is empty
        emit dataChanged(packetModel->createIndex(0,0), packetModel->createIndex(packetModel->rowCount() - 1, packetModel->columnCount() - 1));
}

FilterEngine *PacketSortFilterProxyModel::getFilterEngine() const
{
    return filterEngine;
}

QHash<QString, QString> PacketSortFilterProxyModel::getConfiguration()
{
    QHash<QString, QString> ret;

    ret.insert(GuiConst::STATE_PACKET_PROXY_FILTERS_ENABLE, filterEngine->getFilteringEnabled() ? QString("1") : QString("0"));
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
    ret.insert(GuiConst::STATE_PACKET_PROXY_SORTING_ORDER, QString::number(static_cast<int>(sortOrder())));

    return ret;
}

void PacketSortFilterProxyModel::setConfiguration(const QHash<QString, QString> &conf)
{
    bool ok = false;
    if (conf.contains(GuiConst::STATE_PACKET_PROXY_FILTERS_ENABLE)) {
        int val = conf.value(GuiConst::STATE_PACKET_PROXY_FILTERS_ENABLE).toInt(&ok);
        if (ok && (val == 0 || val == 1)) {
            filterEngine->setFilteringEnabled(val == 1);
        }
    }

    if (conf.contains(GuiConst::STATE_PACKET_PROXY_EQUALITY_ENABLE)) {
        int val = conf.value(GuiConst::STATE_PACKET_PROXY_EQUALITY_ENABLE).toInt(&ok);
        if (ok && (val == 0 || val == 1)) {
            equalityViewEnabled = (val == 1);
        }
    }

    if (conf.contains(GuiConst::STATE_PACKET_PROXY_EQUALITY_TARGET_COLUMNS)) {
        QStringList equalList = conf.value(GuiConst::STATE_PACKET_PROXY_EQUALITY_TARGET_COLUMNS).split(GuiConst::STATE_FIELD_SEPARATOR,
                                                                                               #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                                                                                       Qt::SkipEmptyParts
                                                                                               #else
                                                                                                       QString::SkipEmptyParts
                                                                                               #endif
                                                                                                       );
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
                if (ok && (order == static_cast<int>(Qt::AscendingOrder) || order == static_cast<int>(Qt::DescendingOrder))) {
                    sort(val, static_cast<Qt::SortOrder>(order));
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

    if (filterEngine != nullptr) {
        QSharedPointer<Packet> pac = packetModel->getPacket(sourceRow);
        if (!pac.isNull())
            ret = filterEngine->evaluate(pac);
    }
    return ret;
}

SortFilterProxyStateObj::SortFilterProxyStateObj(PacketSortFilterProxyModel *proxyModel) :
    proxyModel(proxyModel)
{
    //name = metaObject()->className();
}

SortFilterProxyStateObj::~SortFilterProxyStateObj()
{

}

void SortFilterProxyStateObj::run()
{
    QHash<QString, QString> conf;
    FilterItemsList filterList = proxyModel->filterEngine->getItems();
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        // saving sorting/filtering information
        writer->writeStartElement(GuiConst::STATE_FILTER_SORT);
        writer->writeAttribute(GuiConst::STATE_FILTER_EXPR,proxyModel->filterEngine->getExpressionStr());
        conf = proxyModel->getConfiguration();
        QHashIterator<QString, QString> hi(conf);
        while (hi.hasNext()) {
            hi.next();
            writer->writeAttribute(hi.key(), hi.value());
        }

        writer->writeStartElement(GuiConst::STATE_FILTER_ITEMS);
        for (int i = 0; i < filterList.size(); i++) {
            writer->writeStartElement(GuiConst::STATE_FILTER_ITEM);
            QSharedPointer<FilterItem> item = filterList.at(i);
            conf = item->getConfiguration();
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
            QString expr;
            if (attrList.hasAttribute(GuiConst::STATE_FILTER_EXPR)) {
                expr = attributes.value(GuiConst::STATE_FILTER_EXPR).toString();
            }

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
                    QSharedPointer<FilterItem> item = QSharedPointer<FilterItem>(FilterItem::instanciateFromConf(conf));
                    if (!item.isNull() && item->isValid()) {
                        filterList.append(item);
                    }

                    proxyModel->filterEngine->setItems(filterList);
                    proxyModel->filterEngine->assertExpr(expr);
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
