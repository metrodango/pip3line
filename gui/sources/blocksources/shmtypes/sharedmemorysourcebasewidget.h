#ifndef SHAREDMEMORYSOURCEBASEWIDGET_H
#define SHAREDMEMORYSOURCEBASEWIDGET_H

#include <QWidget>

namespace Ui {
    class SharedMemorySourceBaseWidget;
}

class SharedMemorySource;

class SharedMemorySourceBaseWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SharedMemorySourceBaseWidget(SharedMemorySource *source, QWidget *parent = nullptr);
        ~SharedMemorySourceBaseWidget();
        void addWidget(QWidget * gui);
    public slots:
        void onTypechanged(int index);
    private:
        Q_DISABLE_COPY(SharedMemorySourceBaseWidget)
        Ui::SharedMemorySourceBaseWidget *ui;
        SharedMemorySource *source;
};

#endif // SHAREDMEMORYSOURCEBASEWIDGET_H
