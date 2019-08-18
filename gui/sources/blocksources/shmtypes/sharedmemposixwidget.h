#ifndef SHAREDMEMPOSIXWIDGET_H
#define SHAREDMEMPOSIXWIDGET_H

#include <QWidget>

namespace Ui {
    class SharedMemPosixWidget;
}

class SharedMemPosix;

class SharedMemPosixWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SharedMemPosixWidget(SharedMemPosix *connector, QWidget *parent = nullptr);
        ~SharedMemPosixWidget() override;
    public slots:
        void onSubmit();
    private:
        Q_DISABLE_COPY(SharedMemPosixWidget)
        Ui::SharedMemPosixWidget *ui;
        SharedMemPosix *connector;
};

#endif // SHAREDMEMPOSIXWIDGET_H
