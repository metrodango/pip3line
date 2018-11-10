#include "jsonview.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QDebug>
#include <sources/bytesourceabstract.h>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include "shared/sendtomenu.h"
#include "guihelper.h"
#include "shared/guiconst.h"
#include <transformabstract.h>
#include <cmath>
using namespace GuiConst;

const QString JsonItem::OBJECT_STR = "[Object]";
const QString JsonItem::ARRAY_STR = "[Array]";
const QString JsonItem::NULL_STR = "[null]";

JsonItem::JsonItem(const QJsonValue &nvalue, JsonItem *parent, const QString &itemName) :
    itemName(itemName),
    _parent(parent)
{
    setValue(nvalue);
}

JsonItem::~JsonItem()
{
    clearChildren();
    _parent = nullptr;
}

bool JsonItem::hasChildren()
{
    return children.size() > 0;
}

int JsonItem::childrenCount()
{
    return children.size();
}

JsonItem* JsonItem::parent()
{
    return _parent;
}

void JsonItem::setValue(const QJsonValue &nvalue)
{
    clearChildren();
    if (nvalue.isArray()) {
        QJsonArray array = nvalue.toArray();
        for (int i = 0; i < array.size(); i++) {
            JsonItem * newItem = new(std::nothrow) JsonItem(array.at(i), this, QString::number(i));
            children.append(newItem);
        }
        valueType = ARRAY;
    } else if (nvalue.isObject()) {
        QJsonObject obj = nvalue.toObject();
        QList<QString> names = obj.keys();

        for (int i = 0; i < names.size(); i++) {
            JsonItem * newItem = new(std::nothrow) JsonItem(obj.value(names.at(i)), this, names.at(i));
            children.append(newItem);
        }
        valueType = OBJECT;
    } else {
        value = nvalue;
        valueType = OTHER;
    }
}

QVariant JsonItem::data(int column) const
{
    QVariant ret;
    // only two column
    if (column == 0) {
            ret = itemName;
    } else {
        if (valueType == ARRAY) {
            QString str = ARRAY_STR;
            if (children.isEmpty())
                str.append(" (empty)");
            ret = str;
        } else if (valueType == OBJECT) {
            QString str = OBJECT_STR;
            if (children.isEmpty())
                str.append(" (empty)");
            ret = str;
        } else if (value.isBool()){
            ret = value.toBool() ? QString("true") : QString("false");
        } else if (value.isDouble()) {
            ret = QString::number(value.toInt(0));
            if (ret == 0) {
                double out = value.toDouble();
                double intprt;
                if (std::modf(out,&intprt) == 0.0) { // this is an integer
                    ret = QString::number(static_cast<qint64>(intprt));
                } else {
                    ret = QString::number(out,'f',10);
                }
            }
        } else if (value.isString()){
            ret = value.toString();
        } else if (value.isNull()) {
            ret = NULL_STR;
        } else { // undefined
            ret = QString("undefined");
        }
    }
    return ret;
}

int JsonItem::row() const
{
    int ret = 0;

    if (_parent != nullptr)
        ret = _parent->indexOf(const_cast<JsonItem *>(this));

    return ret;
}

JsonItem* JsonItem::child(int row)
{
    JsonItem* ret = nullptr;

    if (row >= 0 && row < children.size()) {
        ret = children.at(row);
    } else {
        qCritical() << QObject::tr("[JsonItem* JsonItem::child] Index Out-Of-Bound: %1").arg(row);
    }
    return ret;
}

int JsonItem::indexOf(JsonItem * item)
{
    return children.indexOf(item);
}

QJsonValue JsonItem::toJson()
{
    QJsonValue ret;
    if (valueType == ARRAY) {
        QJsonArray array;
        for (int i = 0; i < children.size(); i++) {
            array.append(children.at(i)->toJson());
        }
        ret = QJsonValue(array);
    } else if (valueType == OBJECT) {
        QJsonObject obj;
        for (int i = 0; i < children.size(); i++) {
            JsonItem * child = children.at(i);
            obj.insert(child->getName() , child->toJson());
        }
        ret = QJsonValue(obj);
    } else {
        ret = value;
    }

    return ret;
}

QString JsonItem::getName() const
{
    return itemName;
}

void JsonItem::clearChildren()
{
    for (int i = 0; i < children.size(); i++) {
        delete children.at(i);
    }

    children.clear();
}

JsonItem::Type JsonItem::getValueType() const
{
    return valueType;
}

const QString JsonModel::ROOT_STR = "(root)";

JsonModel::JsonModel(const QJsonDocument &jsonDoc, QObject *parent) :
    QAbstractItemModel(parent)
{
    readonly = false;
    root = new(std::nothrow) JsonItem(QJsonValue(), nullptr, QString("root Item"));
    if (root == nullptr) {
        qFatal("Cannot allocate memory for root JsonItem X{");
    }
    setJsonDoc(jsonDoc);
}

JsonModel::~JsonModel()
{
    delete root;
}

int JsonModel::columnCount(const QModelIndex &) const
{
    int ret = 2;

    return ret;
}

int JsonModel::rowCount(const QModelIndex &parent) const
{
    int ret = 0;
    JsonItem *parentItem = nullptr;
    if (!parent.isValid()) {
        parentItem = root;
    } else if (parent.column() == 0) {
        parentItem = static_cast<JsonItem*>(parent.internalPointer());
    }

    if (parentItem != nullptr)
        ret = parentItem->childrenCount();

    return ret;
}

QVariant JsonModel::data(const QModelIndex &index, int role) const
{
    QVariant ret;
    if (index.isValid()) {
        int col = index.column();
        if (role == Qt::DisplayRole) {
            ret = static_cast<JsonItem*>(index.internalPointer())->data(col);
        } else if (role == Qt::ForegroundRole) {
            if (col == 0) {
                ret = QVariant(GlobalsValues::JSON_KEY_COLOR);
            } else {
                if (static_cast<JsonItem*>(index.internalPointer())->getValueType() == JsonItem::OTHER) {
                    ret = QVariant(GlobalsValues::JSON_VALUE_COLOR);
                } else {
                    ret = QVariant(GlobalsValues::JSON_TYPE_COLOR);
                }
            }
        }
    }
    return ret;
}

bool JsonModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool ret = false;
    if (index.isValid() && role == Qt::EditRole && !readonly) {
        JsonItem* item = static_cast<JsonItem*>(index.internalPointer());
        QJsonValue jval;
        switch(value.type()) {
            case QVariant::Bool:
                jval = QJsonValue(value.toBool());
                break;
            case QVariant::Int:
            case QVariant::UInt:
                jval = QJsonValue(value.toInt());
                break;
            case QVariant::LongLong:
            case QVariant::ULongLong:
                jval = QJsonValue(value.toLongLong());
                break;
            case QVariant::Double:
                jval = QJsonValue(value.toDouble());
                break;
            case QVariant::String:
                jval = QJsonValue(value.toString());
                break;
            default:
                break;
        }
        if (!jval.isNull()) {
            item->setValue(jval);
            emit jsonUpdated();
            ret = true;
        }

    }
    return ret;
}

Qt::ItemFlags JsonModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
              return nullptr;

    JsonItem* item = static_cast<JsonItem*>(index.internalPointer());
    JsonItem::Type vtype = item->getValueType();
    if (vtype == JsonItem::OTHER && !readonly) {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractItemModel::flags(index);
}

QVariant JsonModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0) {
            return QString("Name/Index");
        } else {
            return QString("Value");
        }
    }
    return QVariant();
}

QModelIndex JsonModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex ret;

    JsonItem * parentitem = nullptr;

    if (!parent.isValid()) {
        parentitem = root;
    } else {
        parentitem = static_cast<JsonItem*>(parent.internalPointer());
    }

    if (parentitem != nullptr && row < parentitem->childrenCount()) {
        JsonItem * child = parentitem->child(row);
        if (child != nullptr) {
            ret = createIndex(row,column, reinterpret_cast<void *>(child));
        }
    }
    return ret;
}

QModelIndex JsonModel::parent(const QModelIndex &index) const
{
    QModelIndex ret;
    if (index.isValid()) {
        JsonItem * item = static_cast<JsonItem*>(index.internalPointer());

        if (item != nullptr && item != root) {
            JsonItem * parentItem = item->parent();
            ret = createIndex(parentItem->row(),0,reinterpret_cast<void *>(parentItem));
        }
    }
    return ret;
}

QJsonDocument JsonModel::getJsonDoc() const
{
    QJsonDocument jsonDoc;
    QJsonValue val = root->toJson();
    val = val.toObject().value(ROOT_STR);
    if (val.isArray()) {
        QJsonArray array = val.toArray();
        jsonDoc = QJsonDocument(array);
    } else if (val.isObject()) {
        QJsonObject obj = val.toObject();
        jsonDoc = QJsonDocument(obj);
    }
    return jsonDoc;
}

void JsonModel::setJsonDoc(const QJsonDocument &jsonDoc)
{
    QJsonObject obj;
    if (jsonDoc.isArray()) {
        obj.insert(ROOT_STR, QJsonValue(jsonDoc.array()));
    } else if (jsonDoc.isObject()) {
        obj.insert(ROOT_STR, QJsonValue(jsonDoc.object()));
    }
    beginResetModel();
    root->setValue(QJsonValue(obj));
  //  qDebug() << "Root tree is set" << root->childrenCount();
  //  QJsonDocument built = getJsonDoc();
  //  qDebug() << "result json:\n" << qPrintable(QString::fromUtf8(built.toJson()));
    endResetModel();
}

bool JsonModel::isNull()
{
    return root->childrenCount() == 0;
}

void JsonModel::setReadonly(bool value)
{
    readonly = value;
}

JsonView::JsonView(ByteSourceAbstract *nbyteSource, GuiHelper *nguiHelper, QWidget *parent, bool takeByteSourceOwnership):
    SingleViewAbstract(nbyteSource, nguiHelper, parent, takeByteSourceOwnership)
{
    tree = new(std::nothrow) QTreeView(this);
    if (tree == nullptr) {
        qFatal("Cannot allocate memory for QTreeView X{");
    }

    model = new(std::nothrow) JsonModel();
    QAbstractItemModel * old = tree->model();
    tree->setModel(model);
    delete old;

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(tree);
    setLayout(layout);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    tree->setSizeAdjustPolicy(QTreeView::AdjustToContents);
    globalContextMenu = nullptr;
    copyMenu = nullptr;
    replaceMenu = nullptr;
    sendToMenu = nullptr;

    model->setReadonly(byteSource->isReadonly());
    connect(byteSource, &ByteSourceAbstract::readOnlyChanged, model, &JsonModel::setReadonly);

    buildContextMenus();
    updateImportExportMenus();
    sourceUpdated();

    connect(tree,&QTreeView::customContextMenuRequested, this, &JsonView::onRightClick);
    connect(byteSource, &ByteSourceAbstract::updated, this, &JsonView::sourceUpdated);
    connect(guiHelper, &GuiHelper::importExportUpdated, this, &JsonView::updateImportExportMenus);
    connect(tree, &QTreeView::expanded, this, &JsonView::onTreeChanges);
    connect(tree,&QTreeView::collapsed, this, &JsonView::onTreeChanges);
    connect(model, &JsonModel::jsonUpdated, this, &JsonView::onManualUpdate);

}

JsonView::~JsonView()
{
    globalContextMenu = nullptr;
    copyMenu = nullptr;
    replaceMenu = nullptr;
    sendToMenu = nullptr;
    delete tree;
    model = nullptr; // deleted by tree
    tree = nullptr;
}

void JsonView::sourceUpdated(quintptr source)
{
    if (source == reinterpret_cast<quintptr>(this))
        return;

    if (isJsonValid()) {
        emit visible();
    } else {
        emit hide();
    }
}

void JsonView::onRightClick(QPoint pos)
{
 //  qDebug() << tr("[JsonView::onRightClick]") << pos.x() << pos.y();
    globalContextMenu->exec(this->mapToGlobal(pos));
}

void JsonView::search(QByteArray , QBitArray )
{
    // not implemented
}

bool JsonView::isJsonValid()
{
    if (byteSource == nullptr) {
        return false;
    }

    QByteArray data = byteSource->getRawData();
    if (data.isEmpty()) {
        return false;
    }

    QJsonParseError error;
    QJsonDocument jdoc = QJsonDocument::fromJson(data, &error);
    if (!jdoc.isNull()) {
        model->setJsonDoc(jdoc);
        disconnect(tree, &QTreeView::expanded, this, &JsonView::onTreeChanges);
        tree->expand(model->index(0,0));
        connect(tree, &QTreeView::expanded, this, &JsonView::onTreeChanges);
        tree->resizeColumnToContents(0);
        restoreTreeState(treeSavedState);
        return true;
    } else {
        return false;
    }
}

void JsonView::updateImportExportMenus()
{
    guiHelper->updateCopyContextMenu(copyMenu);

    QStringList list = guiHelper->getImportExportFunctions();

    replaceMenu->clear();
    QAction * action = new(std::nothrow) QAction(tr("From clipboard as"), replaceMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu clipboard X{");
    }
    action->setDisabled(true);
    replaceMenu->addAction(action);
    action = new(std::nothrow) QAction(UTF8_STRING_ACTION, replaceMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu clipboard X{");
    }

    replaceMenu->addAction(action);
    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), replaceMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu user X{");
        }
        replaceMenu->addAction(action);
    }
}

void JsonView::onCopy(QAction *action)
{
    guiHelper->copyAction(action->text(), getCurrentIndexValue());
}

void JsonView::onReplace(QAction *action)
{
    QModelIndex index = tree->currentIndex();
    JsonItem *item = nullptr;
    if (index.isValid()) {
        item = static_cast<JsonItem*>(index.internalPointer());
        if (item != nullptr) {
            QClipboard *clipboard = QApplication::clipboard();
            QString input = clipboard->text();

            QByteArray rawVal;
            if (action->text() == UTF8_STRING_ACTION) {
                rawVal = input.toUtf8();
            } else {
                TransformAbstract *ta  = guiHelper->getImportExportFunction(action->text());
                if (ta != nullptr) {
                    ta->setWay(TransformAbstract::OUTBOUND);
                    rawVal = ta->transform(input.toUtf8());
                }
            }

            if (!rawVal.isEmpty()) {
                QJsonValue repVal;
                QJsonParseError error;
                QJsonDocument jdoc = QJsonDocument::fromJson(rawVal, &error);
                if (!jdoc.isNull()) {
                    if (jdoc.isArray()) {
                        repVal = QJsonValue(jdoc.array());
                    } else if (jdoc.isObject()) {
                        repVal = QJsonValue(jdoc.object());
                    }

                    item->setValue(repVal);
                } else {
                    repVal = QJsonValue(QString::fromUtf8(rawVal));
                }

                item->setValue(repVal);
                byteSource->setData(model->getJsonDoc().toJson());
            }
        }
    }
}

void JsonView::onSendToTriggered(QAction *action)
{
    sendToMenu->processingAction(action, getCurrentIndexValue());
}

void JsonView::onManualUpdate()
{
    byteSource->setData(model->getJsonDoc().toJson());
}

void JsonView::onTreeChanges()
{
    tree->resizeColumnToContents(0);
    treeSavedState = getTreeState();
    // qDebug() << "[JsonView::onTreeChanges] " << qPrintable(QString::fromUtf8(treeSavedState.toJson()));
}

QJsonDocument JsonView::getTreeState()
{
    QJsonDocument doc;
    QModelIndex root = model->index(0, 0, tree->rootIndex());
    QJsonObject obj = getState(root);
    doc.setObject(obj);
    // qDebug() << "[JsonView::getTreeState] " << qPrintable(QString::fromUtf8(doc.toJson()));
    return doc;
}

QJsonObject JsonView::getState(const QModelIndex &index)
{
    QJsonObject ret;
    JsonItem* item = static_cast<JsonItem*>(index.internalPointer());
    if (item != nullptr) {
        int count = item->childrenCount();
        if (item->getValueType() == JsonItem::ARRAY) {
            ret.insert(GuiConst::STATE_TYPE, JsonItem::ARRAY);
        } else if (item->getValueType() == JsonItem::OBJECT) {
            ret.insert(GuiConst::STATE_TYPE, JsonItem::OBJECT);
        } else {
            return ret;
        }

        QJsonObject data;
        for (int i = 0; i < count; i++) {
            QModelIndex child = model->index(i,0,index);
            if (tree->isExpanded(child)) {
                item = static_cast<JsonItem*>(child.internalPointer());
                data.insert(item->getName(), getState(child));
            }
        }

        ret.insert(GuiConst::STATE_DATA, data);
    }
    return ret;
}

void JsonView::restoreTreeState(const QJsonDocument &doc)
{
    if (!doc.isEmpty()) {
        if (doc.isObject()) {
            treeSavedState = doc;
            if (!model->isNull()) {

                QJsonObject obj = doc.object();
                restoreState(obj,model->index(0,0,tree->rootIndex()));
            }
        } else {
            qCritical() << tr("[JsonView::restoreTreeState] Json root needs to be an object");
        }
    }
}

void JsonView::restoreState(const QJsonObject &obj, const QModelIndex &index)
{
    if (obj.contains(GuiConst::STATE_TYPE)) {
        JsonItem* item = static_cast<JsonItem*>(index.internalPointer());
        if (item != nullptr) {
            int type = obj.value(GuiConst::STATE_TYPE).toInt();
            if (item->getValueType() == type) {
                tree->expand(index);
            }

            if (obj.contains(GuiConst::STATE_DATA)) {
                QJsonObject data = obj.value(GuiConst::STATE_DATA).toObject();
                if (!data.isEmpty()) {
                    for (int i = 0; i < item->childrenCount(); i++) {
                        JsonItem* child = item->child(i);
                        if (data.contains(child->getName())) {
                            restoreState(data.value(child->getName()).toObject(),model->index(i,0,index));
                        }
                    }
                }
            }
        }
    }
}

QHash<QString, QString> JsonView::getConfiguration()
{
    QHash<QString, QString> ret = SingleViewAbstract::getConfiguration();
    ret.insert(GuiConst::STATE_JSON_STATE, QString::fromUtf8(treeSavedState.toJson()));
    return ret;
}

void JsonView::setConfiguration(QHash<QString, QString> conf)
{
    SingleViewAbstract::setConfiguration(conf);
    if (conf.contains(GuiConst::STATE_JSON_STATE)) {
        QString data = conf.value(GuiConst::STATE_JSON_STATE);
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &error);
        if (!doc.isNull()) {
            restoreTreeState(doc);
        } else {
            qCritical() << tr("[JsonView::setConfiguration] Parsing error: %1").arg(error.errorString());
        }
    }
}

void JsonView::buildContextMenus()
{
    sendToMenu = new(std::nothrow) SendToMenu(guiHelper, tr("Send value to"));
    if (sendToMenu == nullptr) {
        qFatal("Cannot allocate memory for sendToMenu X{");
    }
    connect(sendToMenu, &SendToMenu::triggered, this, &JsonView::onSendToTriggered);

    copyMenu = new(std::nothrow) QMenu(tr("Copy value as"));
    if (copyMenu == nullptr) {
        qFatal("Cannot allocate memory for copyMenu X{");
    }
    connect(copyMenu, &QMenu::triggered, this, &JsonView::onCopy);

    replaceMenu = new(std::nothrow) QMenu(tr("Replace value"));
    if (replaceMenu == nullptr) {
        qFatal("Cannot allocate memory for replaceMenu X{");
    }
    connect(replaceMenu, &QMenu::triggered, this, &JsonView::onReplace);

    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == nullptr) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
    }

    QAction * action = new(std::nothrow)QAction(QString("Expand All"));
    if (action == nullptr) {
        qFatal("Cannot allocate memory for expandAllAction X{");
    }

    connect(action, &QAction::triggered, tree, &QTreeView::expandAll);

    globalContextMenu->addAction(action);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(replaceMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(copyMenu);

    action = new(std::nothrow)QAction(QString("Collapse All"));
    if (action == nullptr) {
        qFatal("Cannot allocate memory for collapseAllAction X{");
    }

    globalContextMenu->addSeparator();
    globalContextMenu->addAction(action);

    connect(action, &QAction::triggered, tree, &QTreeView::collapseAll);
}

QByteArray JsonView::getCurrentIndexValue()
{
    QByteArray ret;
    QModelIndex index = tree->currentIndex();
    JsonItem *item = nullptr;
    if (index.isValid()) {
        item = static_cast<JsonItem*>(index.internalPointer());
        if (item != nullptr) {
            if (item->getValueType() == JsonItem::OTHER) {
                ret = item->data(1).toString().toUtf8();
            } else { // array or object
                QJsonDocument jsonDoc;
                QJsonValue val = item->toJson();
                if (val.isArray()) {
                    QJsonArray array = val.toArray();
                    jsonDoc = QJsonDocument(array);
                } else if (val.isObject()) {
                    QJsonObject obj = val.toObject();
                    jsonDoc = QJsonDocument(obj);
                }
                ret = jsonDoc.toJson();
            }
        }
    }

    return ret;
}

QJsonDocument JsonView::getTreeSavedState() const
{
    // qDebug() << "[JsonView::getTreeSavedState] " << qPrintable(QString::fromUtf8(treeSavedState.toJson()));
    return treeSavedState;
}
