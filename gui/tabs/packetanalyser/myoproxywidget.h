#ifndef MYOPROXYWIDGET_H
#define MYOPROXYWIDGET_H

#include <QWidget>

namespace Ui {
    class MYOProxyWidget;
}

class MYOProxy;

class MYOProxyWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit MYOProxyWidget(MYOProxy *proxy, QWidget *parent = nullptr);
        ~MYOProxyWidget();
        void setServerWidget(QWidget * gui);
        void setClientWidget(QWidget * gui);
    public slots:
        void onServerChanged(int index);
        void onClientChanged(int index);
    private:
        Ui::MYOProxyWidget *ui;
        MYOProxy *proxy;
};

#endif // MYOPROXYWIDGET_H
