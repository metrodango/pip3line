#ifndef TLSCLIENTLISTENER_H
#define TLSCLIENTLISTENER_H

#include "ipblockssources.h"
#include <QHash>
#include <QSslSocket>
#include "connectiondetails.h"

class TLSClientListener : public IPBlocksSources
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit TLSClientListener(QHostAddress hostAddress = TLSClientListener::DEFAULT_ADDRESS, quint16 hostPort = TLSClientListener::DEFAULT_PORT, QObject *parent = nullptr);
        ~TLSClientListener();
        QString getName();
        QString getDescription();
        bool isStarted();
        QWidget *getAdditionnalCtrls(QWidget * parent = nullptr);
    public slots:
        void sendBlock(Block *block);
        bool startListening();
        void stopListening();
        void setSpecificConnection(int sourceId, ConnectionDetails cd);
        void onConnectionClosed(int cid);
        void createConnection();
    private slots:
        void dataReceived();
        void onSslErrors(const QList<QSslError> & errors);
        void onError(QAbstractSocket::SocketError error);
        void onSSLModeChange(QSslSocket::SslMode mode);
        void onClientDeconnection();
        void onTLSStarted();
        void onPlainStarted();
        void onTLSUpdated(bool enabled);
    private:
        Q_DISABLE_COPY(TLSClientListener)
        void internalUpdateConnectionsInfo();
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        QHash<QSslSocket *, int> sockets;
        QHash<int,int> mapExtSourcesToLocal;
        QThread workerThread;
        bool running;
        QString actualID;
        QHash<int, ConnectionDetails > specificConnectionsData;
};

#endif // TLSCLIENTLISTENER_H
