#ifndef DTLSCLIENTLISTENER_H
#define DTLSCLIENTLISTENER_H

#include <QtGlobal>
#if QT_FEATURE_dtls != -1

#include "ipblockssources.h"
#include <QHash>
#include <QDtls>
#include <QUdpSocket>
#include "connectiondetails.h"
#include <QSharedPointer>

class DTLSClientListener : public IPBlocksSources
{
        Q_OBJECT
    public:
        static const QString ID;
        explicit DTLSClientListener(QHostAddress hostAddress = DTLSClientListener::DEFAULT_ADDRESS,
                                    quint16 hostPort = DTLSClientListener::DEFAULT_PORT,
                                    QObject *parent = nullptr);
        ~DTLSClientListener();
        QString getName() override;
        QString getDescription() override;
        bool isStarted() override;
        QWidget *getAdditionnalCtrls(QWidget * parent = nullptr) override;
        int getTargetIdFor(int sourceId) override;
        bool getUseSrcPorts() const;
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(const  QHash<QString, QString> &conf) override;
    public slots:
        void sendBlock(Block *block) override;
        bool startListening() override;
        void stopListening() override;
        void onConnectionClosed(int cid) override;
        void createConnection();
        void setUseSrcPorts(bool value);
    protected:
        QWidget *requestGui(QWidget *parent) override;
    private slots:
        void dataReceived();
        void onError(QAbstractSocket::SocketError error);
        void handshakeTimeout();
        void pskRequired(QSslPreSharedKeyAuthenticator *auth);
    private:
        Q_DISABLE_COPY(DTLSClientListener)
        void internalUpdateConnectionsInfo() override;
        static const quint16 DEFAULT_PORT;
        static const QHostAddress DEFAULT_ADDRESS;
        static const int MAX_DTLS_DATAGRAM_SIZE_HARD;
        static const QString USE_SRC_PORT;
        QHash<QUdpSocket *, QSharedPointer<ConnectionDetails> > sockets;
        QHash<int,int> mapExtSourcesToLocal;
        QThread workerThread;
        bool running;
        QHash<QUdpSocket *, QList<QByteArray> > waitingHandshakePackets;
        bool useSrcPorts;
};

#endif // dtls
#endif // DTLSCLIENTLISTENER_H
