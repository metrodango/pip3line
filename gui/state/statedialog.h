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
        explicit StateDialog(QWidget *parent = nullptr);
        ~StateDialog();

    public slots:
        void log(QString message, QString source, Pip3lineConst::LOGLEVEL level);
        void update();
    private:
        Q_DISABLE_COPY(StateDialog)
        Ui::StateDialog *ui;
};

#endif // STATEDIALOG_H
