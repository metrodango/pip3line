#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QItemSelection>
#include <QList>
#include <QObject>

class FilterItem;
class QEvent;


namespace Ui {
    class FilterDialog;
}

class PacketSortFilterProxyModel;

class FilterDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit FilterDialog(PacketSortFilterProxyModel *sortFilterProxyModel, QWidget *parent = 0);
        ~FilterDialog();
    public slots:
        void onAdd();
        void updateList();
        void onDeleteItem(QString name);
        void onFilterToggled(bool toggled);
        void onItemSelected(QItemSelection selection);
        void onClear();
        void onSave();
    private:
        FilterItem getCurrentConfItem();
        bool eventFilter(QObject *obj, QEvent *event);
        Ui::FilterDialog *ui;
        PacketSortFilterProxyModel *sortFilterProxyModel;
        QList<FilterItem> list;
};

#endif // FILTERDIALOG_H
