#ifndef DTLSCLIENTLISTENERWIDGET_H
#define DTLSCLIENTLISTENERWIDGET_H

#include <QWidget>

namespace Ui {
    class DtlsClientListenerWidget;
}

class DTLSClientListener;

class DtlsClientListenerWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit DtlsClientListenerWidget(DTLSClientListener * blocksource, QWidget *parent = nullptr);
        ~DtlsClientListenerWidget();
    public slots:
        void onSrcPortUseToggle(bool value);

    private:
        Ui::DtlsClientListenerWidget *ui;
        DTLSClientListener * blocksource;
};

#endif // DTLSCLIENTLISTENERWIDGET_H
