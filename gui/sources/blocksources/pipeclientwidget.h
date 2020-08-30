#ifndef PIPECLIENTWIDGET_H
#define PIPECLIENTWIDGET_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

class PipesModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        explicit PipesModel(QObject *parent = nullptr);
        ~PipesModel() override;
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    public slots:
        void refresh();
    private:
        Q_DISABLE_COPY(PipesModel)
        struct PipePropperties {
            QString path;
            QString perms;
            QString processOwner;
            QString bonus;
            quint64 inode;
        };
        struct ProcessProperties {
            #if defined(Q_OS_LINUX)
            pid_t pid;
            #elif defined(Q_OS_WIN32)
            int pid;
            #endif
            QString desc;
        };
        static const int COLUMN_COUNT;
        static const QStringList headersList;
        QMultiHash<quint64, ProcessProperties> getProcList();
        QList<PipePropperties> pipesList;
};

namespace Ui {
    class PipeClientWidget;
}

class PipeClientWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit PipeClientWidget(QWidget *parent = nullptr);
        ~PipeClientWidget() override;
    signals:
        void newPipeName(const QString &mess);
    public slots:
        void onModelUpdated();
        void onSelectPipe();
        void onSelectionChanged(const QItemSelection &selection);
        void onNamedPipeChanged(const QString &value);
    private:
        Q_DISABLE_COPY(PipeClientWidget)
        Ui::PipeClientWidget *ui;
        PipesModel * model;
        QSortFilterProxyModel *proxyModel;
};

#endif // PIPECLIENTWIDGET_H
