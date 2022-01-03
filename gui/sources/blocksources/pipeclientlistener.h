#ifndef PIPECLIENTLISTENER_H
#define PIPECLIENTLISTENER_H

#include "blockssource.h"
#include <QLocalSocket>
#include <QObject>

class PipeClientListener : public BlocksSource
{
        Q_OBJECT
    public:
        static const QString ID;
        struct PipeConnection {
                QDateTime creationTimeStamp;
                int sid;
                QString pipeName;
        };
        PipeClientListener(QObject *parent = nullptr);
        ~PipeClientListener() override;
        QString getName() override;
        QString getDescription() override;
        bool isStarted() override;
        QHash<QString, QString> getConfiguration() override;
        void setConfiguration(const  QHash<QString, QString> &conf) override;
        QWidget *getAdditionnalCtrls(QWidget * parent = nullptr) override;
        int getTargetIdFor(int sourceId) override;
    public slots:
        void sendBlock(Block * block) override;
        bool startListening() override;
        void stopListening() override;
        void setPipeName(const QString & name);
        void createConnection();
    private slots:
        void dataReceived();
        void onSocketError(QLocalSocket::LocalSocketError socketError);
        void onClientDeconnection();
    private:
        Q_DISABLE_COPY(PipeClientListener)
        virtual QWidget *requestGui(QWidget * parent = nullptr) override;
        void internalUpdateConnectionsInfo() override;
        QString pipeName;
        QHash<QLocalSocket *, PipeConnection> sockets;
        QHash<int,int> mapExtSourcesToLocal;
        QThread workerThread;
        bool running;
        QString actualID;
};

#endif // PIPECLIENTLISTENER_H
