#ifndef DTLSSERVERLISTENERWIDGET_H
#define DTLSSERVERLISTENERWIDGET_H

#include <QWidget>

namespace Ui {
    class DtlsServerListenerWidget;
}

class DtlsServerListener;

class DtlsServerListenerWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit DtlsServerListenerWidget(DtlsServerListener *blocksource, QWidget *parent = nullptr);
        ~DtlsServerListenerWidget();
    public slots:
        void onEnableClientVerifiation(bool value);
    private:
        Ui::DtlsServerListenerWidget *ui;
        DtlsServerListener *blocksource;
};

#endif // DTLSSERVERLISTENERWIDGET_H
