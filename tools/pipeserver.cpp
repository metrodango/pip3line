/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "pipeserver.h"
#include <commonstrings.h>
#include "shared/guiconst.h"
using namespace GuiConst;

InternalLocalSocketServer::InternalLocalSocketServer(QObject *parent):
    QLocalServer(parent)
{
}

void InternalLocalSocketServer::incomingConnection(quintptr socketDescriptor)
{
    emit newClient(socketDescriptor);
}

LocalSocketProcessor::LocalSocketProcessor(TransformMgmt *tFactory, quintptr socketDescr, QObject *parent):
    StreamProcessor(tFactory,parent)
{
    socketDescriptor = socketDescr;
}

LocalSocketProcessor::~LocalSocketProcessor()
{
}

void LocalSocketProcessor::run()
{
    QLocalSocket *socket = new(std::nothrow) QLocalSocket();
    if (socket != nullptr) {
        socket->setSocketDescriptor(static_cast<qintptr>(socketDescriptor));

        emit status(tr("New Client for %1").arg(socketDescriptor), PIPE_SOCKET);

        connect(socket, &QLocalSocket::disconnected, this, &LocalSocketProcessor::stop);
        in  = socket;
        if (out == nullptr)
            out = in;

        StreamProcessor::run();
        emit status(tr("Client for %1 disconnected").arg(socketDescriptor), PIPE_SOCKET);
        emit finished(this);
    }  else {
        qFatal("Cannot allocate memory pipe socket X{");
    }
}

PipeServer::PipeServer(TransformMgmt *tFactory, QObject *parent) :
    ServerAbstract(tFactory)
{
    pipeServer = new(std::nothrow) InternalLocalSocketServer(parent);
    if (pipeServer != nullptr) {
        connect(pipeServer, &InternalLocalSocketServer::newClient, this, &PipeServer::processingNewClient);
    }  else {
        qFatal("Cannot allocate memory pipe server X{");
    }
}

PipeServer::~PipeServer()
{
    if (pipeServer != nullptr) {
        pipeServer->close();
        delete pipeServer;
    }
}

void PipeServer::setPipeName(const QString &name)
{
    pipeName = name;
}

bool PipeServer::startServer()
{
    bool ret = false;
    if (pipeServer != nullptr) {
        ret = pipeServer->listen(pipeName);
        if (ret) {
            logStatus(tr("Started for %1").arg(pipeName), PIPE_SERVER);
        } else {
            logError(tr("Error while starting:\n%1").arg(pipeServer->errorString()),PIPE_SERVER);
        }
    }
    return ret;
}

void PipeServer::stopServer()
{
    if (pipeServer != nullptr) {
        if (pipeServer->isListening()) {
            pipeServer->close();
            ServerAbstract::stopServer();
            logStatus(tr("Stopped (%1)").arg(pipeName),PIPE_SERVER);
        }
    }
}

QString PipeServer::getLastError()
{
    return pipeServer != nullptr ? pipeServer->errorString() : "Null pipe server" ;
}

void PipeServer::processingNewClient(quintptr socketDescriptor)
{
    LocalSocketProcessor * processor = new(std::nothrow) LocalSocketProcessor(transformFactory, socketDescriptor,this);
    if (processor != nullptr) {
        connect(processor, &LocalSocketProcessor::finished, this, &PipeServer::processorFinished, Qt::QueuedConnection);
        connect(processor, &LocalSocketProcessor::error, this, &PipeServer::logError);
        connect(processor, &LocalSocketProcessor::status, this, &PipeServer::logStatus);
        //connect(this, &PipeServer::newTransformChain, processor, qOverload<const QString &>(&LocalSocketProcessor::setTransformsChain));
        connect(this, SIGNAL(newTransformChain(QString)), processor, SLOT(setTransformsChain(QString)));

        processor->setOutput(output);

        confLocker.lock();
        processor->setTransformsChain(tconf);
        processor->setSeparator(separator);
        processor->setDecoding(decode);
        processor->setEncoding(encode);
        clientProcessor.append(processor);
        confLocker.unlock();

        processor->start();
    } else {
        qFatal("Cannot allocate memory pipe processor X{");
    }
}

void PipeServer::processorFinished(LocalSocketProcessor * target)
{
    confLocker.lock();
    ProcessingStats preStats = target->getStats();
    stats += preStats;
    if (clientProcessor.contains(target)) {
        clientProcessor.removeAll(target);
    } else {
        qWarning("[Pipe server] Processor finished, but target not found in the list ... T_T");
    }

    confLocker.unlock();
    target->deleteLater();
}

QString PipeServer::getServerType()
{
    return PIPE_SERVER;
}
