#ifndef UDPCLIENTLISTENER_H
#define UDPCLIENTLISTENER_H

#include "ipblockssources.h"
#include <QTimer>
#include <QUdpSocket>
#include "connectiondetails.h"

class UdpClientListener : public IPBlocksSources
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit UdpClientListener(QHostAddress hostAddress = UdpClientListener::DEFAULT_ADDRESS, quint16 hostPort = UdpClientListener::DEFAULT_PORT, QObject *parent = nullptr);
        ~UdpClientListener();
        QString getName();
        QString getDescription();
        bool isStarted();
    public slots:
        void sendBlock(Block *block);
        bool startListening();
        void stopListening();
    private slots:
        void dataReceived();
        void checkTimeouts();
    private:
        Q_DISABLE_COPY(UdpClientListener)
        void internalUpdateConnectionsInfo();
        static const int MAX_UDP_DATAGRAM_SIZE_HARD;
        static const int MAX_UDP_DATAGRAM_SIZE_SOFT;
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        QHash<QUdpSocket *, ConnectionDetails> udpSockets;
        QHash<int,int> mapExtSourcesToLocal;
        QThread workerThread;
        bool running;
        QTimer connectionsTimer;
};

#endif // UDPCLIENTLISTENER_H
