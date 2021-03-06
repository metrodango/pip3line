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
        explicit DefaultDialog(QWidget *parent = nullptr);
        ~DefaultDialog();
        void setMainWidget(QWidget * widget);
    private:
        Q_DISABLE_COPY(DefaultDialog)
        Ui::DefaultDialog *ui;
};

#endif // DEFAULTDIALOG_H
