#ifndef CONNECTIONSWIDGET_H
#define CONNECTIONSWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include "blockssource.h"

class TargetModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit TargetModel(BlocksSource * source, QObject *parent = nullptr);
        ~TargetModel() override;
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        void setConnections(QList<Target<BlocksSource *> > connections);
        int size();

    private:
        Q_DISABLE_COPY(TargetModel)
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
        explicit ConnectionsWidget(BlocksSource * source, QWidget *parent = nullptr);
        ~ConnectionsWidget() override;
    public slots:
        void connectionsUpdated();
    private:
        Q_DISABLE_COPY(ConnectionsWidget)
        Ui::ConnectionsWidget *ui;
        BlocksSource * source;
        TargetModel *model;
};

#endif // CONNECTIONSWIDGET_H
