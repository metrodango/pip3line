#ifndef BASEBLOCKSOURCEWIDGET_H
#define BASEBLOCKSOURCEWIDGET_H

#include <QTabWidget>

namespace Ui {
    class BaseBlockSourceWidget;
}

class BlocksSource;

class BaseBlockSourceWidget : public QTabWidget
{
        Q_OBJECT
    public:
        explicit BaseBlockSourceWidget(BlocksSource *bs, QWidget *parent = 0);
        ~BaseBlockSourceWidget();
        void insertWidgetInGeneric(int index, QWidget * widget);
        void setTLSWidget(QWidget * widget);
    signals:
        void tlsEnabled(bool checked);
    private slots:
        void onTLSToggled(bool checked);
        void onInboundTransformModified();
        void onOutboundTransformModified();
        void onInboundTransformWidgetDDestroyed();
        void onOutboundTransformWidgetDDestroyed();
    private:
        Ui::BaseBlockSourceWidget *ui;
        BlocksSource *bsource;
        QWidget * sslgui;
        QWidget * inboundTranformWidget;
        QWidget * outboundTranformWidget;
};

#endif // BASEBLOCKSOURCEWIDGET_H
