#ifndef DEFAULTCONTROLGUI_H
#define DEFAULTCONTROLGUI_H

#include <QWidget>

namespace Ui {
    class DefaultControlGui;
}

class DefaultControlGui : public QWidget
{
        Q_OBJECT

    public:
        explicit DefaultControlGui(QWidget *parent = nullptr);
        ~DefaultControlGui();
        void setStateStopped(bool startState);
        void appendWidget(QWidget *widget);
        void setConfButtonVisible(bool visible);

    public slots:
        void receiveStart();
        void receiveStop();
    signals:
        void start();
        void stop();
        void reset();
        void requestConfPanel();
    private slots:
        void onStartStop();

    private:
        Q_DISABLE_COPY(DefaultControlGui)
        Ui::DefaultControlGui *ui;
        bool stateStopped{true};
};

#endif // DEFAULTCONTROLGUI_H
