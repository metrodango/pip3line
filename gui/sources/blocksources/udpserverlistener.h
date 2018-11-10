#ifndef UDPSERVERLISTENER_H
#define UDPSERVERLISTENER_H

#include "ipblockssources.h"
#include <QUdpSocket>
#include <QList>
#include <QTimer>
#include "connectiondetails.h"

class QWidget;
class QThread;

class UdpServerListener : public IPBlocksSources
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit UdpServerListener(QHostAddress hostAddress = UdpServerListener::DEFAULT_ADDRESS, quint16 hostPort = UdpServerListener::DEFAULT_PORT, QObject *parent = nullptr);
        ~UdpServerListener();
        QString getName();
        QString getDescription();
        bool isStarted();
    public slots:
        void sendBlock(Block *block);
        bool startListening();
        void stopListening();
    private slots:
        void packetReceived();
        void checkTimeouts();
    private:
        Q_DISABLE_COPY(UdpServerListener)
        void internalUpdateConnectionsInfo();
        static const int MAX_DATAGRAM_SIZE;
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        QUdpSocket *udpSocket;
        QThread serverThread;
        QList<ConnectionDetails> clients;
        QTimer connectionsTimer;
};

#endif // UDPSERVERLISTENER_H
