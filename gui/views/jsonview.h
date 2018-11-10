#ifndef JSONVIEW_H
#define JSONVIEW_H

#include <QJsonDocument>
#include <QObject>
#include <QTreeView>
#include <QJsonValue>
#include <QSharedPointer>
#include "singleviewabstract.h"

class QMenu;
class SendToMenu;

class JsonItem
{
    public:
        enum Type {
            ARRAY,
            OBJECT,
            OTHER
        };
        explicit JsonItem(const QJsonValue &value, JsonItem * parent = nullptr, const QString &itemName = QString());
        ~JsonItem();
        bool hasChildren();
        int childrenCount();
        JsonItem *parent();
        void setValue(const QJsonValue &value);
        QVariant data(int column) const;
        int row() const;
        JsonItem *child(int row);
        int indexOf(JsonItem *item);
        QJsonValue toJson();
        QString getName() const;
        JsonItem::Type getValueType() const;
    private:
        Q_DISABLE_COPY(JsonItem)
        static const QString OBJECT_STR;
        static const QString ARRAY_STR;
        static const QString NULL_STR;
        void clearChildren();
        QString itemName;
        QJsonValue value;
        JsonItem * _parent;
        QList<JsonItem *> children;
        JsonItem::Type valueType;
};

class JsonModel : public QAbstractItemModel
{
        Q_OBJECT
    public:
        explicit JsonModel(const QJsonDocument &jsonDoc = QJsonDocument(), QObject *parent = nullptr);
        ~JsonModel() override;
        int	columnCount(const QModelIndex &parent = QModelIndex()) const override;
        int	rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex	parent(const QModelIndex &index) const override;
        bool setData(const QModelIndex &index, const QVariant &value, int role) override;
        QJsonDocument getJsonDoc() const;
        void setJsonDoc(const QJsonDocument &jsonDoc);
        bool isNull();
    public slots:
        void setReadonly(bool value);
    signals:
        void jsonUpdated();
    private:
        Q_DISABLE_COPY(JsonModel)
        static const QString ROOT_STR;
        JsonItem *root;
        bool readonly;
};

class JsonView : public SingleViewAbstract
{
        Q_OBJECT
    public:
        explicit JsonView(ByteSourceAbstract *byteSource, GuiHelper *guiHelper, QWidget *parent = nullptr, bool takeByteSourceOwnership = false);
        ~JsonView() override;
        QJsonDocument getTreeSavedState() const;
        void restoreTreeState(const QJsonDocument &doc);
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(QHash<QString, QString> conf) override;
    public slots:
        void sourceUpdated(quintptr source = 0);
        void onRightClick(QPoint pos);
        void search(QByteArray item, QBitArray mask) override;
        bool isJsonValid();
        void updateImportExportMenus();
        void onCopy(QAction * action);
        void onReplace(QAction * action);
        void onSendToTriggered(QAction * action);
        void onManualUpdate();
        void onTreeChanges();
    signals:
        void hide();
        void visible();
    private:
        Q_DISABLE_COPY(JsonView)
        QJsonDocument getTreeState();
        QJsonObject getState(const QModelIndex &index);
        void restoreState(const QJsonObject &obj, const QModelIndex &index);
        void buildContextMenus();
        QByteArray getCurrentIndexValue();
        QMenu * globalContextMenu;
        QMenu * copyMenu;
        QMenu * replaceMenu;
        SendToMenu * sendToMenu;
        QTreeView *tree;
        JsonModel *model;
        QJsonDocument treeSavedState;
};

#endif // JSONVIEW_H
