/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MODULETRANSFORMWIDGET_H
#define MODULETRANSFORMWIDGET_H

#include "libtransform_global.h"
#include <QWidget>
#include <QList>

namespace Ui {
class ModuleTransformWidget;
}
class ScriptTransformAbstract;
class QMenu;
class QAction;

#include <QAbstractTableModel>
#include <QHash>
#include <QPoint>


class ParametersItemModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit ParametersItemModel( QObject *parent = nullptr);
        ~ParametersItemModel();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        Qt::ItemFlags flags(const QModelIndex &index) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
        void addBlankRow();
        bool removeRows(int row, int count, const QModelIndex &parent);
        void setParameters(QHash<QByteArray, QByteArray> newParameters);
        QHash<QByteArray, QByteArray> getParameters();
    signals:
        void parametersChanged();
    private:
        Q_DISABLE_COPY(ParametersItemModel)
        QList<QString> parametersNames;
        QList<QString> parametersValues;

};

class LIBTRANSFORMSHARED_EXPORT ModuleTransformWidget : public QWidget
{
        Q_OBJECT
        
    public:
        explicit ModuleTransformWidget(ScriptTransformAbstract *transform, QWidget *parent = nullptr);
        ~ModuleTransformWidget();
    public slots:
        void reloadConf();
    private slots:
        void onChooseFile();
        void onMakePersistent(bool checked);
        void onAddParameter();
        void onParametersUpdated();
        void onAutoReload(bool val);
        void customMenuRequested(QPoint pos);
        void onMenuAction(QAction * action);
    private:
        Q_DISABLE_COPY(ModuleTransformWidget)
        static const QString MENU_DELETE;
        bool eventFilter(QObject *sender, QEvent *event);
        Ui::ModuleTransformWidget *ui;
        ScriptTransformAbstract *transform;
        ParametersItemModel *model;
        QMenu *tableMenu;
        bool reloadingParams;
};

#endif // MODULETRANSFORMWIDGET_H
