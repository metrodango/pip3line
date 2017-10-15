#ifndef PACKETANALYSEROPTIONSDIALOG_H
#define PACKETANALYSEROPTIONSDIALOG_H

#include <QAbstractListModel>
#include <QDialog>
#include <QItemSelection>
#include <QModelIndex>
#include <QStringList>
#include <commonstrings.h>

namespace Ui {
class PacketAnalyserOptionsDialog;
class TransformDisplayConf;
}

class PacketModelAbstract;
class PacketSortFilterProxyModel;
class QTableView;
class GuiHelper;
class QListWidgetItem;
class SourcesOrchestatorAbstract;


class ColumnModel : public QAbstractTableModel
{
        Q_OBJECT
    public:
        enum COLUMN_INDEXES {
            COLUMN_NAME = 0,
            COLUMN_HIDDEN = 1,
            COLUMN_EQUALITY = 2,
            COLUMN_DELETE = 3
        };
        explicit ColumnModel(GuiHelper *guiHelper,
                             PacketModelAbstract *mainModel,
                             PacketSortFilterProxyModel *proxyModel,
                             QTableView *tableView,
                             QObject *parent = 0);
        ~ColumnModel();
        int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
        int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
        QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
        QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
        bool setData(const QModelIndex &index, const QVariant &value, int role);
        Qt::ItemFlags flags(const QModelIndex &index) const;
        int getDefaultColumnwidth(int column);
    public slots:
        void onColumnsUpdated();
    private:

        static const QStringList colNames;
        GuiHelper *guiHelper;
        PacketModelAbstract *mainModel;
        PacketSortFilterProxyModel *proxyModel;
        QTableView *tableView;

};

class PacketAnalyserOptionsDialog : public QDialog
{
        Q_OBJECT
    public:
        explicit PacketAnalyserOptionsDialog(GuiHelper *guiHelper,
                                             PacketModelAbstract *mainModel,
                                             PacketSortFilterProxyModel *proxyModel,
                                             QTableView *tableView,
                                             QWidget *parent = 0);
        ~PacketAnalyserOptionsDialog();
    private slots:
        void onItemSelected(QItemSelection index);
        void onItemSelected(int index);
        void onAddNewColumn();
        void onInboundButtonToggled(bool checked);
        void onTextFormatToggled(bool checked);
        void onInfoClicked();
        void onEqualityBackgroundClicked();
        void onEqualityForegroundClicked();
        void onIndexClicked(const QModelIndex &index);
        void onColumnsUpdated();
    signals:
        void setEqualityColumn(int index, bool equalityOps);
    private:
        void setFormatVisible(bool visible, Pip3lineConst::OutputFormat format = Pip3lineConst::TEXTFORMAT);

        PacketModelAbstract *mainModel;
        PacketSortFilterProxyModel *proxyModel;
        QTableView *tableView;
        GuiHelper *guiHelper;
        Ui::PacketAnalyserOptionsDialog *ui;
        Ui::TransformDisplayConf *uiTransform;
        QWidget *currentGui;
        QHash<QWidget *, QListWidgetItem *> itemsWidgets;
        ColumnModel * colModel;
};

#endif // PACKETANALYSEROPTIONSDIALOG_H
