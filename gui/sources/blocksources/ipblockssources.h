#ifndef IPBLOCKSSOURCES_H
#define IPBLOCKSSOURCES_H

#include "blockssource.h"
#include <QHostAddress>
#include <QSslConfiguration>
#include <QHash>

class SslConf;

class IPBlocksSources : public BlocksSource
{
        Q_OBJECT
    public:
        explicit IPBlocksSources(QHostAddress hostAddress = QHostAddress::LocalHost, quint16 hostPort = 443, QObject *parent = nullptr);
        virtual ~IPBlocksSources();
        virtual QHostAddress getHostAddress() const;
        virtual quint16 getHostPort() const;
        QString getHostname() const;
        QSslConfiguration getSslConfiguration();
        void setSslConfiguration(QSslConfiguration conf);
        QWidget * getSSLConfGui(QWidget *parent = nullptr);
        virtual QList<QSslCertificate> getPeerCerts();
        virtual QString getCurrentCipher();
        virtual QHash<QString, QString> getConfiguration();
        virtual void setConfiguration(const  QHash<QString, QString> &conf);

    public slots:
        void setHostname(const QString &name);
        virtual void setHostPort(const quint16 &port);
        virtual void setHostAddress(const QHostAddress &addr);
    protected:
        Q_DISABLE_COPY(IPBlocksSources)
        virtual QWidget *requestGui(QWidget * parent = nullptr);
        QHostAddress hostAddress;
        quint16 hostPort;
        QString hostName;
        SslConf *sslConfiguration;
};

#endif // IPBLOCKSSOURCES_H
