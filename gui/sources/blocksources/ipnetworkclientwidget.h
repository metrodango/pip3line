#ifndef IPNETWORKCLIENTWIDGET_H
#define IPNETWORKCLIENTWIDGET_H

#include <QHostInfo>
#include <QTimer>
#include <QWidget>
#include <commonstrings.h>

namespace Ui {
    class IPNetworkClientWidget;
}

class IPBlocksSources;

class IPNetworkClientWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit IPNetworkClientWidget(IPBlocksSources *nlistener, QWidget *parent = nullptr);
        ~IPNetworkClientWidget() override;
    signals:
        void log(QString mess, QString source, Pip3lineConst::LOGLEVEL level);
        void newSNI(QString value);
    public slots:
        void onTlsToggled(bool enabled);
    private slots:
        void onPortChanged(int value);
        void onIPChanged(const QString &value);
        void onTargetchanged();
        void onResolutionTimerExpired();
        void lookupFinished(QHostInfo info);
    private:
        Q_DISABLE_COPY(IPNetworkClientWidget)
        Ui::IPNetworkClientWidget *ui;
        IPBlocksSources * listener;
        QTimer resolutionTimer;
};

#endif // IPNETWORKCLIENTWIDGET_H
