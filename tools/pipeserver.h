/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef PIPESERVER_H
#define PIPESERVER_H

#include <QLocalServer>
#include <QLocalSocket>
#include <QMutex>
#include <transformmgmt.h>
#include <QSemaphore>
#include <QObject>
#include "streamprocessor.h"
#include "serverabstract.h"

class LocalSocketProcessor : public StreamProcessor
{
        Q_OBJECT
        friend class PipeServer;
    public:
        ~LocalSocketProcessor();
        void run();

    signals:
        void finished(LocalSocketProcessor *);
    private :
        Q_DISABLE_COPY(LocalSocketProcessor)
        explicit LocalSocketProcessor(TransformMgmt * tFactory, quintptr socketDescriptor, QObject * parent = 0);
        quintptr socketDescriptor;
};

class InternalLocalSocketServer : public QLocalServer
{
    Q_OBJECT
        friend class PipeServer;
    signals:
        void newClient(quintptr socketDescriptor);
    private:
        Q_DISABLE_COPY(InternalLocalSocketServer)
        explicit InternalLocalSocketServer(QObject *parent = 0);
        void incomingConnection(quintptr socketDescriptor);
};

class PipeServer : public ServerAbstract
{
        Q_OBJECT
    public:
        explicit PipeServer(TransformMgmt *tFactory, QObject *parent = 0);
        ~PipeServer();
        bool startServer();
        void stopServer();
        QString getLastError();
        QString getServerType();
    public slots:
        void setPipeName(const QString & name);
    private slots:
        void processingNewClient(quintptr socketDescriptor);
        void processorFinished(LocalSocketProcessor *target);
    private:
        Q_DISABLE_COPY(PipeServer)
        InternalLocalSocketServer * pipeServer;
        QString pipeName;
};

#endif // PIPESERVER_H
