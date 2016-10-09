#ifndef TLSSERVERLISTENER_H
#define TLSSERVERLISTENER_H

#include "ipblockssources.h"
#include <QSslSocket>
#include <QHash>
#include <QTcpServer>
#include <QMutex>
#include <QReadWriteLock>
#include "connectiondetails.h"

class QWidget;
class QThread;
class SocksProxyHelper;

class BaseTcpServer : public QTcpServer
{
        Q_OBJECT
    public:
        explicit BaseTcpServer(QObject *parent = 0);
        ~BaseTcpServer();
#if QT_VERSION >= 0x050000
    signals:
        void newClient(qintptr socketDescriptor);
    private slots:
        void incomingConnection(qintptr socketDescriptor);
#else
    signals:
        void newClient(int socketDescriptor);
    private slots:
        void incomingConnection(int socketDescriptor);
#endif
};


class TLSServerListener : public IPBlocksSources
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit TLSServerListener(QHostAddress hostAddress = TLSServerListener::DEFAULT_ADDRESS, quint16 hostPort = TLSServerListener::DEFAULT_PORT, QObject *parent = 0);
        ~TLSServerListener();
        QString getName();
        QString getDescription();
        bool isStarted();
        bool isSocks5Proxy() const;
        void setSocks5Proxy(bool value);

    public slots:
        void sendBlock(Block *block);
        bool startListening();
        void stopListening();
        void onConnectionClosed(int cid);
    signals:
        void newConnectionData(int sourceId, ConnectionDetails cd);
    private slots:
        void dataReceived();
#if QT_VERSION >= 0x050000
        void handlingClient(qintptr socketDescriptor);
#else
        void handlingClient(int socketDescriptor);
#endif
        void onSslErrors(const QList<QSslError> & errors);
        void onError(QAbstractSocket::SocketError error);
        void onSSLModeChange(QSslSocket::SslMode mode);
        void onClientDeconnection();
        void onTLSUpdated(bool enabled);
    private:
        void internalUpdateConnectionsInfo();
        bool startingTLS(QSslSocket *socket);
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        void handlingDisconnect(QSslSocket *socket);
        QSslSocket *serverSocket;
        BaseTcpServer *server;
        QThread serverThread;

        QHash<QSslSocket *, int> clients;
        QHash<QSslSocket *, SocksProxyHelper *> clientsProxyNeeded;
        QString actualID;
        bool socksProxy;
        QList<quint16> tlsPorts;
};

#endif // TLSSERVERLISTENER_H
