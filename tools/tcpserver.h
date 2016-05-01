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
#if QT_VERSION >= 0x050000
                                    qintptr socketDescr,
#else
                                    int socketDescr,
#endif
                                    QObject * parent = 0);
        static const QString TCP_SOCKET;
#if QT_VERSION >= 0x050000
        qintptr socketDescriptor;
#else
        int socketDescriptor;
#endif

        QTcpSocket * socket;
};

class InternalTcpServer : public QTcpServer
{
        Q_OBJECT
        friend class TcpServer;
    signals:
#if QT_VERSION >= 0x050000
        void newClient(qintptr socketDescriptor);
#else
        void newClient(int socketDescriptor);
#endif

    private:
        Q_DISABLE_COPY(InternalTcpServer)
        explicit InternalTcpServer(QObject *parent = 0);
#if QT_VERSION >= 0x050000
        void incomingConnection(qintptr socketDescriptor);
#else
        void incomingConnection(int socketDescriptor);
#endif
};

class TcpServer : public ServerAbstract
{
        Q_OBJECT
    public:
        explicit TcpServer(TransformMgmt *tFactory, QObject *parent = 0);
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
#if QT_VERSION >= 0x050000
        void processingNewClient(qintptr socketDescriptor);
#else
        void processingNewClient(int socketDescriptor);
#endif
    private:
        Q_DISABLE_COPY(TcpServer)
       // QMutex confLocker;
        InternalTcpServer *tcpServer;
        QString IP;
        int port;
};

#endif // TCPSERVER_H
