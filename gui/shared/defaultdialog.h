#ifndef DEFAULTDIALOG_H
#define DEFAULTDIALOG_H

#include <QDialog>

namespace Ui {
    class DefaultDialog;
}

class DefaultDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit DefaultDialog(QWidget *parent = 0);
        ~DefaultDialog();
        void setMainWidget(QWidget * widget);
    private:
        Ui::DefaultDialog *ui;
};

#endif // DEFAULTDIALOG_H
