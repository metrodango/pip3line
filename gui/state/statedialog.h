#ifndef STATEDIALOG_H
#define STATEDIALOG_H

#include <QDialog>
#include <commonstrings.h>

namespace Ui {
    class StateDialog;
}

class StateDialog : public QDialog
{
        Q_OBJECT

    public:
        explicit StateDialog(QWidget *parent = 0);
        ~StateDialog();

    public slots:
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
        void update();
    private:
        Ui::StateDialog *ui;
};

#endif // STATEDIALOG_H
