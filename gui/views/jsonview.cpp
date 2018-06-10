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
            ret = ARRAY_STR;
        } else if (valueType == OBJECT) {
            ret = OBJECT_STR;
        } else if (value.isBool()){
            ret = value.toBool() ? QString("true") : QString("false");
        } else if (value.isDouble()) {
            ret = QString::number(value.toDouble());
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
        if (role == Qt::DisplayRole) {
            ret = static_cast<JsonItem*>(index.internalPointer())->data(index.column());
        }
    }
    return ret;
}

Qt::ItemFlags JsonModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
              return 0;

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

    if (parentitem != nullptr) {
        JsonItem * child = parentitem->child(row);
        if (child != nullptr) {
            ret = createIndex(row,column, (void *)child);
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
            ret = createIndex(parentItem->row(),0,(void *)parentItem);
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

    tree->setContextMenuPolicy(Qt::CustomContextMenu);

    globalContextMenu = nullptr;
    copyMenu = nullptr;
    replaceMenu = nullptr;
    sendToMenu = nullptr;

    buildContextMenus();
    updateImportExportMenus();
    sourceUpdated();

    connect(tree,&QTreeView::customContextMenuRequested, this, &JsonView::onRightClick);
    connect(byteSource, &ByteSourceAbstract::updated, this, &JsonView::sourceUpdated);
    connect(guiHelper, &GuiHelper::importExportUpdated, this, &JsonView::updateImportExportMenus);
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
    if (source == (quintptr) this)
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

    if (model == nullptr || tree == nullptr) {
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
        tree->expand(model->index(0,0));
        return true;
    } else {
        return false;
      //  qDebug() << tr("Json parsing error:") << error.errorString();
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
        return;
    }
    action->setDisabled(true);
    replaceMenu->addAction(action);
    action = new(std::nothrow) QAction(UTF8_STRING_ACTION, replaceMenu);
    if (action == nullptr) {
        qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu clipboard X{");
        return;
    }

    replaceMenu->addAction(action);
    for (int i = 0; i < list.size(); i++) {
        action = new(std::nothrow) QAction(list.at(i), replaceMenu);
        if (action == nullptr) {
            qFatal("Cannot allocate memory for action updateImportExportMenus replaceMenu user X{");
            return;
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
    qDebug() << tr("replacing");
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
                qDebug() << tr("Got value");
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

void JsonView::buildContextMenus()
{
    sendToMenu = new(std::nothrow) SendToMenu(guiHelper, tr("Send value to"));
    if (sendToMenu == nullptr) {
        qFatal("Cannot allocate memory for sendToMenu X{");
        return;
    }
    connect(sendToMenu, &SendToMenu::triggered, this, &JsonView::onSendToTriggered);

    copyMenu = new(std::nothrow) QMenu(tr("Copy value as"));
    if (copyMenu == nullptr) {
        qFatal("Cannot allocate memory for copyMenu X{");
        return;
    }
    connect(copyMenu, &QMenu::triggered, this, &JsonView::onCopy);

    replaceMenu = new(std::nothrow) QMenu(tr("Replace value"));
    if (replaceMenu == nullptr) {
        qFatal("Cannot allocate memory for replaceMenu X{");
        return;
    }
    connect(replaceMenu, &QMenu::triggered, this, &JsonView::onReplace);

    globalContextMenu = new(std::nothrow) QMenu();
    if (globalContextMenu == nullptr) {
        qFatal("Cannot allocate memory for globalContextMenu X{");
        return;
    }

    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(replaceMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(sendToMenu);
    globalContextMenu->addSeparator();
    globalContextMenu->addMenu(copyMenu);
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
