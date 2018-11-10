#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QItemSelection>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QValidator>
#include  "filteritem.h"

class QEvent;
class FilterEngine;


namespace Ui {
    class FilterDialog;
}

class PacketSortFilterProxyModel;

class NameValidator : public QValidator
{
        Q_OBJECT
    public:
        explicit NameValidator(QObject *parent = nullptr);
        ~NameValidator();
        QValidator::State validate(QString &input, int &pos) const;
    private:
        Q_DISABLE_COPY(NameValidator)
};

class CIDValidator : public QValidator
{
        Q_OBJECT
    public:
        explicit CIDValidator(QObject *parent = Q_NULLPTR);
        ~CIDValidator();
        QValidator::State validate(QString &input, int &pos) const;
    private:
        Q_DISABLE_COPY(CIDValidator)
};

class FilterDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit FilterDialog(PacketSortFilterProxyModel *sortFilterProxyModel, QWidget *parent = nullptr);
        ~FilterDialog();
    public slots:
        void updatedList();
        void validate();
    private slots:
        void onAdd();
        void onDeleteItem(QString name);
        void onFilterToggled(bool toggled);
        void onItemSelected(QItemSelection selection);
        void onClear();
        void onSave();
        void onColumnsUpdated();
        void onColumnSelected(int index);
    private:
        Q_DISABLE_COPY(FilterDialog)
        void initUi();
        QSharedPointer<FilterItem> getCurrentConfItem();
        bool eventFilter(QObject *obj, QEvent *event);
        QTimer validationTimer;
        Ui::FilterDialog *ui;
        PacketSortFilterProxyModel *sortFilterProxyModel;
        FilterItemsList list;
        FilterEngine * filterEngine;
        NameValidator nameValidator;
        CIDValidator cidValidator;
};

#endif // FILTERDIALOG_H
