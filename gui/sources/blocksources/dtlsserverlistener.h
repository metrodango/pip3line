#ifndef DTLSSERVERLISTENER_H
#define DTLSSERVERLISTENER_H

#include "ipblockssources.h"
#include <QHostAddress>
#include <QUdpSocket>
#include <QDtls>
#include <QThread>
#include <QList>
#include <QTimer>
#include <QSharedPointer>
#include "connectiondetails.h"



class DtlsServerListener : public IPBlocksSources
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit DtlsServerListener(QHostAddress hostAddress = DtlsServerListener::DEFAULT_ADDRESS, quint16 hostPort = DtlsServerListener::DEFAULT_PORT, QObject *parent = nullptr);
        ~DtlsServerListener() override;
        QString getName() override;
        QString getDescription() override;
        bool isStarted() override;
        int getTargetIdFor(int sourceId) override;
        bool getDtlsVerificationEnabled() const;
        void setDtlsVerificationEnabled(bool value);
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(const  QHash<QString, QString> &conf) override;
    public slots:
        void sendBlock(Block *block) override;
        bool startListening() override;
        void stopListening() override;
    private slots:
        void packetReceived();
        void checkTimeouts();
        void handleDtlsHandshakeTimeOut();
    protected:
        QWidget *requestGui(QWidget *parent) override;
    private:
        Q_DISABLE_COPY(DtlsServerListener)
        void internalUpdateConnectionsInfo() override;
        static const int MAX_DATAGRAM_SIZE;
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        static const QString DTLS_VERIFICATION_ENABLED;
        QUdpSocket *udpSocket;
        QThread serverThread;
        ConnectionDetailsList clients;
        QTimer connectionsTimer;
        QDtlsClientVerifier verifier;
        QList<QPair<QHostAddress, quint16> > verifiedclient;
        int getClientIndex(const QHostAddress &clientAddress, const quint16 &clientPort);
        bool dtlsVerificationEnabled;
};

#endif // DTLSSERVERLISTENER_H
