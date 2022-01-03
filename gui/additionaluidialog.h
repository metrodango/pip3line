#ifndef ADDITIONALUIDIALOG_H
#define ADDITIONALUIDIALOG_H

#include <QDialog>

namespace Ui {
    class AdditionalUIDialog;
}

class AdditionalUIDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit AdditionalUIDialog(QWidget *childWidget, QWidget *parent = nullptr);
        ~AdditionalUIDialog();

    private:
        Ui::AdditionalUIDialog *ui;
};

#endif // ADDITIONALUIDIALOG_H
