/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <transformmgmt.h>
#include <QSemaphore>
#include "streamprocessor.h"
#include "serverabstract.h"

class TcpSocketProcessor : public StreamProcessor
{
        Q_OBJECT
        friend class TcpServer;
    public:
        ~TcpSocketProcessor();
        void run();
    signals:
        void finished(TcpSocketProcessor *);
    private :
        Q_DISABLE_COPY(TcpSocketProcessor)
	explicit TcpSocketProcessor() {}
        explicit TcpSocketProcessor(TransformMgmt * tFactory,
                                    qintptr socketDescr,
                                    QObject * parent = nullptr);
        static const QString TCP_SOCKET;
        qintptr socketDescriptor;

        QTcpSocket * socket;
};

class InternalTcpServer : public QTcpServer
{
        Q_OBJECT
        friend class TcpServer;
    signals:
        void newClient(qintptr socketDescriptor);

    private:
        Q_DISABLE_COPY(InternalTcpServer)
        explicit InternalTcpServer(QObject *parent = nullptr);
        void incomingConnection(qintptr socketDescriptor);
};

class TcpServer : public ServerAbstract
{
        Q_OBJECT
    public:
        explicit TcpServer(TransformMgmt *tFactory, QObject *parent = nullptr);
        ~TcpServer();
        bool startServer();
        void stopServer();
        QString getLastError();
        QString getServerType();
    public slots:
        void setIP(const QString &ip);
        void setPort(int port);
    private slots:
        void processorFinished(TcpSocketProcessor *);
        void processingNewClient(qintptr socketDescriptor);
    private:
        Q_DISABLE_COPY(TcpServer)
       // QMutex confLocker;
        InternalTcpServer *tcpServer;
        QString IP;
        quint16 port;
};

#endif // TCPSERVER_H
