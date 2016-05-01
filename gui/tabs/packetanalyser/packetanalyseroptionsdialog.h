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
        void onHiddenToggled(bool checked);
        void onEqualityToggled(bool checked);
        void onItemSelected(QItemSelection index);
        void onItemSelected(int index);
        void onAddNewColumn();
        void onDeleteColumn(const QString &name);
        void onInboundButtonToggled(bool checked);
        void onTextFormatToggled(bool checked);
        void onInfoClicked();
    signals:
        void hideColumn(int index, bool hide);
        void setEqualityColumn(int index, bool equalityOps);
    private:
        void setFormatVisible(bool visible, Pip3lineConst::OutputFormat format = Pip3lineConst::TEXTFORMAT);
        int addItem(const QString &name);
        int addItems(const QStringList &names);
        PacketModelAbstract *mainModel;
        PacketSortFilterProxyModel *proxyModel;
        QTableView *tableView;
        GuiHelper *guiHelper;
        Ui::PacketAnalyserOptionsDialog *ui;
        Ui::TransformDisplayConf *uiTransform;
        QWidget *currentGui;
        QHash<QWidget *, QListWidgetItem *> itemsWidgets;
};

#endif // PACKETANALYSEROPTIONSDIALOG_H
