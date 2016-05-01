#ifndef STATESTATUSWIDGET_H
#define STATESTATUSWIDGET_H

#include <QWidget>
#include <commonstrings.h>

namespace Ui {
    class StateStatusWidget;
}

class StateStatusWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit StateStatusWidget(QWidget *parent = 0);
        ~StateStatusWidget();
    public slots:
        void log(QString, QString, Pip3lineConst::LOGLEVEL);
        void update();
        void setMessage(QString message);
    private:
        Ui::StateStatusWidget *ui;
};

#endif // STATESTATUSWIDGET_H
