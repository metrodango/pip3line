#ifndef IPNETWORKSERVERWIDGET_H
#define IPNETWORKSERVERWIDGET_H

#include <QHostAddress>
#include <QWidget>
#include "shared/guiconst.h"
#include <commonstrings.h>

namespace Ui {
class IPNetworkServerWidget;
}

class IPBlocksSources;

class IPNetworkServerWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit IPNetworkServerWidget(IPBlocksSources *nlistener, QWidget *parent = nullptr);
        ~IPNetworkServerWidget();
    public slots:
        void onTlsToggled(bool enabled);
    private slots:
        void refreshIPs();
        void onPortChanged(int port);
        void onIPChanged(const QString &value);
    signals:
        void log(QString mess, QString source, Pip3lineConst::LOGLEVEL level);
    private:
        Q_DISABLE_COPY(IPNetworkServerWidget)
        IPBlocksSources * listener;
        Ui::IPNetworkServerWidget *ui;
};

#endif // IPNETWORKSERVERWIDGET_H
