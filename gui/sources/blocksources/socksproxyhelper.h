#ifndef SOCKSPROXYHELPER_H
#define SOCKSPROXYHELPER_H

#include <QByteArray>
#include <QHostAddress>

class SocksProxyHelper
{
    public:
        enum CONNECTION_STATE {INVALID, NEED_AUTH_DATA, AUTHENTICATED, DONE, REJECTED};
        SocksProxyHelper(QHostAddress serverAddr, quint16 serverPort);
        ~SocksProxyHelper() {}
        QByteArray processRequest(QByteArray data);
        CONNECTION_STATE getState() const;
        QHostAddress getHost() const;
        quint16 getPort() const;
    private:
        Q_DISABLE_COPY(SocksProxyHelper)
        CONNECTION_STATE connectionState;
        QHostAddress host;
        quint16 port;
        QHostAddress serverAddr;
        quint16 serverPort;
};

#endif // SOCKSPROXYHELPER_H
