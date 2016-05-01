#ifndef UDPSERVERLISTENER_H
#define UDPSERVERLISTENER_H

#include "ipblockssources.h"
#include <QUdpSocket>
#include <QList>
#include <QTimer>
#include "udpclient.h"

class QWidget;
class QThread;

class UdpServerListener : public IPBlocksSources
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit UdpServerListener(QHostAddress hostAddress = UdpServerListener::DEFAULT_ADDRESS, quint16 hostPort = UdpServerListener::DEFAULT_PORT, QObject *parent = 0);
        ~UdpServerListener();
        QString getName();
        QString getDescription();
        bool isStarted();
    public slots:
        void sendBlock(Block *block);
        bool startListening();
        void stopListening();
        QList<Target<BlocksSource *> > getAvailableConnections();
    private slots:
        void packetReceived();
        void checkTimeouts();
    private:
        static const int MAX_DATAGRAM_SIZE;
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        QUdpSocket *udpSocket;
        QThread serverThread;
        QList<UDPClient> clients;
        QTimer connectionsTimer;
};

#endif // UDPSERVERLISTENER_H
