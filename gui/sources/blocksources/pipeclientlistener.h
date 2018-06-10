#ifndef PIPECLIENTLISTENER_H
#define PIPECLIENTLISTENER_H

#include "blockssource.h"
#include <QLocalSocket>
#include <QObject>

class PipeClientListener : public BlocksSource
{
    public:
        static const QString ID;
        struct PipeConnection {
                QDateTime creationTimeStamp;
                int sid;
                QString pipeName;
        };
        PipeClientListener(QObject *parent = nullptr);
        ~PipeClientListener();
        QString getName();
        QString getDescription();
        bool isStarted();
        QHash<QString, QString> getConfiguration();
        void setConfiguration(QHash<QString, QString> conf);
        QWidget *getAdditionnalCtrls(QWidget * parent = nullptr);
    public slots:
        void sendBlock(Block * block);
        bool startListening();
        void stopListening();
        void setPipeName(const QString & name);
        void createConnection();
    private slots:
        void dataReceived();
        void onSocketError(QLocalSocket::LocalSocketError socketError);
        void onClientDeconnection();
    private:
        virtual QWidget *requestGui(QWidget * parent = nullptr);
        void internalUpdateConnectionsInfo();
        QString pipeName;
        QHash<QLocalSocket *, PipeConnection> sockets;
        QHash<int,int> mapExtSourcesToLocal;
        QThread workerThread;
        bool running;
        QString actualID;
};

#endif // PIPECLIENTLISTENER_H
