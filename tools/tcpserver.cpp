/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "tcpserver.h"
#include <QHostAddress>
#include <commonstrings.h>
#include "shared/guiconst.h"
using namespace GuiConst;

#include <QDebug>


InternalTcpServer::InternalTcpServer(QObject *parent):
    QTcpServer(parent)
{
}

void InternalTcpServer::incomingConnection(qintptr socketDescriptor)
{
    emit newClient(socketDescriptor);
}

const QString TcpSocketProcessor::TCP_SOCKET = "Tcp socket";

TcpSocketProcessor::TcpSocketProcessor(TransformMgmt *tFactory,
                                       qintptr socketDescr,
                                       QObject *parent) :
    StreamProcessor(tFactory,parent)
{
    socketDescriptor = socketDescr;
    socket = nullptr;
}

TcpSocketProcessor::~TcpSocketProcessor()
{
    if (socket != nullptr)
        delete socket;
}

void TcpSocketProcessor::run()
{
    socket = new(std::nothrow) QTcpSocket();

    if (socket != nullptr) {

        bool ret = socket->setSocketDescriptor(socketDescriptor);

        if (!ret) {
            emit error(tr("Could set socket descriptor for the client: %1").arg(socket->errorString()), TCP_SOCKET);
        }
        QString cAddr = socket->peerAddress().toString();
        int port = socket->peerPort();
        emit status(tr("New Client : %1:%2").arg(cAddr).arg(port), TCP_SOCKET);
        connect(socket, SIGNAL(disconnected()), this, SLOT(stop()));
        in  = socket;
        if (out == nullptr)
            out = in;

        StreamProcessor::run();
        emit status(tr("Client disconnected : %1:%2").arg(cAddr).arg(port), TCP_SOCKET);
        emit finished(this);

    } else {
        qFatal("Cannot allocate memory for tcp socket X{");
    }
}

TcpServer::TcpServer(TransformMgmt * tFactory, QObject *parent) :
    ServerAbstract(tFactory)
{
    tcpServer = new(std::nothrow) InternalTcpServer(parent);
    if (tcpServer != nullptr)
        connect(tcpServer, SIGNAL(newClient(qintptr)), this, SLOT(processingNewClient(qintptr)));

    else {
        qFatal("Cannot allocate memory for tcpServer X{");
    }
}

TcpServer::~TcpServer()
{
    if (tcpServer != nullptr) {
        tcpServer->close();
        delete tcpServer;
    }
}

void TcpServer::setIP(const QString &ip)
{
    IP = ip;
}

void TcpServer::processingNewClient(qintptr socketDescriptor)
{
    TcpSocketProcessor * processor = new(std::nothrow) TcpSocketProcessor(transformFactory, socketDescriptor);
    if (processor != nullptr) {
        processor->setOutput(output);

        confLocker.lock();
        processor->setTransformsChain(tconf);
        processor->setSeparator(separator);
        processor->setDecoding(decode);
        processor->setEncoding(encode);
        clientProcessor.append(processor);
        confLocker.unlock();

        connect(processor,SIGNAL(finished(TcpSocketProcessor*)), SLOT(processorFinished(TcpSocketProcessor*)), Qt::QueuedConnection);
        connect(processor,SIGNAL(error(QString,QString)), SLOT(logError(QString,QString)));
        connect(processor,SIGNAL(status(QString,QString)), SLOT(logStatus(QString,QString)));
        connect(this, SIGNAL(newTransformChain(QString)), processor, SLOT(setTransformsChain(QString)));
        processor->start();
    } else {
        qFatal("Cannot allocate memory for TCP processor X{");
    }
}

QString TcpServer::getServerType()
{
    return TCP_SERVER;
}

void TcpServer::stopServer()
{
    if (tcpServer != nullptr) {
        if (tcpServer->isListening()) {
            tcpServer->close();
            ServerAbstract::stopServer();
            emit status(tr("Stopped %1:%2").arg(IP).arg(port), TCP_SERVER);
        }
    }
}

bool TcpServer::startServer()
{
    bool ret = false;
    if (tcpServer != nullptr) {
        ret = tcpServer->listen( QHostAddress(IP), port);
        if (ret) {
            emit status(tr("Started on %1:%2").arg(IP).arg(port), TCP_SERVER);
        } else {
            emit error(tr("Error while starting: %1").arg(tcpServer->errorString()),TCP_SERVER);
        }
    }
    return ret;
}

QString TcpServer::getLastError()
{
    return tcpServer != nullptr ? tcpServer->errorString() : tr("nullptr tcp server");
}

void TcpServer::processorFinished(TcpSocketProcessor * target)
{
    confLocker.lock();

    ProcessingStats preStats = target->getStats();
    stats += preStats;
    if (clientProcessor.contains(target)) {
        clientProcessor.removeAll(target);
    } else {
        qWarning("[TCP server] Processor finished, but target not found in the list ... T_T");
    }

    confLocker.unlock();
    target->deleteLater();
}


void TcpServer::setPort(int nport)
{
    port = nport;
}

