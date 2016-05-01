#ifndef CONNECTIONSWIDGET_H
#define CONNECTIONSWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include "blockssource.h"

class TargetModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit TargetModel(BlocksSource * source, QObject *parent = 0);
        ~TargetModel();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        void setConnections(QList<Target<BlocksSource *> > connections);
        int size();

    private:
        static const int COLUMN_COUNT;
        static const QStringList headersList;
        QList<Target<BlocksSource *> > targetList;
        BlocksSource * source;
};

namespace Ui {
class ConnectionsWidget;
}

class ConnectionsWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ConnectionsWidget(BlocksSource * source, QWidget *parent = 0);
        ~ConnectionsWidget();
    public slots:
        void connectionsUpdated();
    private:
        Ui::ConnectionsWidget *ui;
        BlocksSource * source;
        TargetModel *model;
};

#endif // CONNECTIONSWIDGET_H
