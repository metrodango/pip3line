#include "tlsserverlistener.h"
#include "shared/sslconf.h"
#include <QDir>
#include <QDebug>

BaseTcpServer::BaseTcpServer(QObject *parent) :
    QTcpServer(parent)
{

}

BaseTcpServer::~BaseTcpServer()
{

}

#if QT_VERSION >= 0x050000
void BaseTcpServer::incomingConnection(qintptr socketDescriptor)
#else
void BaseTcpServer::incomingConnection(int socketDescriptor)
#endif
{
    emit newClient(socketDescriptor);
}


const quint16 TLSServerListener::DEFAULT_PORT = 8000;
const QHostAddress TLSServerListener::DEFAULT_ADDRESS = QHostAddress::LocalHost;
const QString TLSServerListener::ID = QString("TCP/TLS server");

TLSServerListener::TLSServerListener(QHostAddress hostAddress, quint16 hostPort, QObject *parent) :
    IPBlocksSources(hostAddress,hostPort, parent),
    server(nullptr)
{
    flags |= REFLEXION_OPTIONS | TLS_OPTIONS | TLS_ENABLED;
    type = SERVER;

    connect(this, SIGNAL(sslChanged(bool)), this, SLOT(onTLSUpdated(bool)));
    setTlsEnable(true);

    QString defaultCertPath = QDir::homePath()
            .append(QDir::separator())
            .append(Pip3lineConst::USER_DIRECTORY)
            .append(QDir::separator())
            .append("cert")
            .append(QDir::separator())
            .append("servercert.pem");
    sslConfiguration->setLocalCert(defaultCertPath);
    sslConfiguration->setLocalKey(defaultCertPath);

    moveToThread(&serverThread);
    serverThread.start();

}

TLSServerListener::~TLSServerListener()
{
    serverThread.quit();
    serverThread.wait();
}

QString TLSServerListener::getName()
{
    return actualID;
}

QString TLSServerListener::getDescription()
{
    return tr("Default TCP/TLS server");
}

bool TLSServerListener::isStarted()
{
    return (server != nullptr && server->isListening());
}

void TLSServerListener::sendBlock(Block *block)
{
    QHashIterator<QSslSocket *, int> i(clients);
    bool dataSend = false;
    while (i.hasNext()) {
        i.next();
        if (i.value() == block->getSourceid()) {    sslConfiguration->setLocalCert(QDir::homePath().append( "/servercert.pem"));
            sslConfiguration->setLocalKey(QDir::homePath().append( "/servercert.pem"));
            QByteArray data = applyOutboundTransform(block->getData());
            if (isB64Blocks()) {
                data = data.toBase64().append(b64BlocksSeparator);
            }
            int size = data.size();
            qint64 written = 0;
            while (written += i.key()->write(data) < size) {
                data = data.mid(written);
            }
            dataSend = true;
        }
    }

    if (!dataSend) {
        qCritical() << tr("[TLSServerListener::sendBlock] Could not found the associated client: %1").arg(block->getSourceid());
    }

    delete block;
}

bool TLSServerListener::startListening()
{
    if (server == nullptr) {
        server = new(std::nothrow) BaseTcpServer();
        if (server == nullptr) {
            qFatal("Cannot allocate memory for BaseTcpServer X{");
        }
#if QT_VERSION >= 0x050000
        connect(server,SIGNAL(newClient(qintptr)), SLOT(handlingClient(qintptr)),Qt::QueuedConnection);
#else
        connect(server,SIGNAL(newClient(int)), SLOT(handlingClient(int)),Qt::QueuedConnection);
#endif
    }

    if (server->isListening()) { // already listening nothing to do
        return true;
    }


    if (!server->listen(hostAddress,hostPort)) {
        emit log(tr("could not start %1").arg(server->errorString()),actualID, Pip3lineConst::LERROR);
        delete server;
        server = nullptr;
        return false;
    }
    emit log(tr("started %1:%2").arg(hostAddress.toString()).arg(hostPort), actualID, Pip3lineConst::LSTATUS);
    emit started();
    return true;
}

void TLSServerListener::stopListening()
{
    if (server != nullptr && server->isListening()) {
        QHashIterator<QSslSocket *, int> i(clients);
        while (i.hasNext()) {
            i.next();
            i.key()->disconnectFromHost();
        }

        clients.clear();

        server->close();
        emit log(tr("stopped %1:%2").arg(hostAddress.toString()).arg(hostPort), actualID, Pip3lineConst::LSTATUS);
        delete server;
        server = nullptr;
        emit stopped();
    }
}

QList<Target<BlocksSource *> > TLSServerListener::getAvailableConnections()
{
    QList<Target<BlocksSource *>> list;

    QHashIterator<QSslSocket *, int> i(clients);
    while (i.hasNext()) {
        i.next();
        QString desc = i.key()->peerAddress().toString();
        if (isTLSEnable())
            desc.append(QString(":%1/TLS").arg(i.key()->peerPort()));
        else
            desc.append(QString(":%1/TCP").arg(i.key()->peerPort()));
        Target<BlocksSource *> tac;
        tac.setDescription(desc);
        tac.setConnectionID(i.value());
        tac.setSource(this);
        list.append(tac);
    }

    return list;
}

void TLSServerListener::dataReceived()
{
    qDebug() << "[TLSServerListener::packetReceived]" << sender();

    QSslSocket * socket = static_cast<QSslSocket *>(sender());
    if (socket != nullptr) {
        if (socket->bytesAvailable() > 0) {
            QByteArray data  = socket->readAll();
            if (isB64Blocks()) {
                processIncomingB64Block(data);
            } else {
                Block * datab = new(std::nothrow) Block(data, clients.value(socket, Block::INVALID_ID));
                if (datab == nullptr) qFatal("Cannot allocate Block for TLSServerListener X{");
                emit blockReceived(datab);
            }
        }
    } else {
        qCritical() << tr("[TLSServerListener::packetReceived] Casted object is nullptr");
    }
}

#if QT_VERSION >= 0x050000
void TLSServerListener::handlingClient(qintptr socketDescriptor)
#else
void TLSServerListener::handlingClient(int socketDescriptor)
#endif
{
    qDebug() << "[TLSServerListener::handlingClient] New client" << socketDescriptor;

    QSslSocket * socket = new(std::nothrow) QSslSocket();
    if (socket == nullptr) {
        qFatal("Cannot allocate memory for QSslSocket X{");
    }

    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(onError(QAbstractSocket::SocketError)),Qt::QueuedConnection);
    connect(socket, SIGNAL(readyRead()), SLOT(dataReceived()),Qt::QueuedConnection);

    socket->setSocketDescriptor(socketDescriptor);

    if (isTLSEnable()) {
        qDebug() << tr("TLS is enabled");
        connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)),Qt::QueuedConnection);
        connect(socket, SIGNAL(modeChanged(QSslSocket::SslMode)),this, SLOT(onSSLModeChange(QSslSocket::SslMode)),Qt::QueuedConnection);
        socket->setSslConfiguration(sslConfiguration->getSslConfiguration());
        socket->startServerEncryption();

        if (socket->waitForEncrypted()) {
            emit log(tr("New %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()),actualID, Pip3lineConst::LSTATUS);
            connect(socket, SIGNAL(disconnected()), this, SLOT(onClientDeconnection()),Qt::QueuedConnection);
            emit updated();
        } else {
            emit log(tr("Client did not send TLS Hello. Closing the connection."),actualID, Pip3lineConst::LERROR);
            socket->disconnectFromHost();
            delete socket;
            socket = NULL;
        }
    } else {
        qDebug() << tr("TLS is not enabled");
        if (socket->waitForConnected()) {
            emit log(tr(" New %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()),actualID, Pip3lineConst::LSTATUS);
            connect(socket, SIGNAL(disconnected()), this, SLOT(onClientDeconnection()),Qt::QueuedConnection);
            emit updated();
        } else {
            emit log(tr("socket error: %1").arg(socket->errorString()),actualID, Pip3lineConst::LERROR);
            delete socket;
            socket = NULL;
        }
    }

    if (socket != nullptr) {
        clients.insert(socket, BlocksSource::newSourceID(this));
    }
}

void TLSServerListener::onSslErrors(const QList<QSslError> &errors)
{
    for (int i = 0; i < errors.size(); i++) {
        emit log(errors.at(i).errorString(), actualID, Pip3lineConst::LWARNING);
    }
}

void TLSServerListener::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "[TLSServerListener::onError]" << error;
    if (error != QAbstractSocket::RemoteHostClosedError) {
        QSslSocket * sobject = static_cast<QSslSocket *>(sender());
        if (sobject != nullptr) {
            emit log(sobject->errorString(), actualID, Pip3lineConst::LERROR);
        } else {
            qDebug() << "[TLSServerListener::onError] a connection was closed by the remote client";
        }
    }
}

void TLSServerListener::onSSLModeChange(QSslSocket::SslMode mode)
{
    QSslSocket * socket = static_cast<QSslSocket *>(sender());
    if (socket != nullptr) {
        qDebug() << "SSL Mode changed for " << socket->peerAddress() << "/" << socket->peerPort() << mode;
    } else {
        qDebug() << "[TLSServerListener::onPeerVerificationError] null casting, mode:" << mode;
    }
}

void TLSServerListener::onClientDeconnection()
{
    QObject *obj = sender();
    if (obj != nullptr) {
        QSslSocket * socket = static_cast<QSslSocket *>(obj);
        if (socket != nullptr) {
            emit log(tr("Disconnection for %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()), actualID, Pip3lineConst::LWARNING);
            if (!b64BlockTempData.isEmpty()) { // just in case
                b64DecodeAndEmit(b64BlockTempData);
                b64BlockTempData.clear();
            }
            if (clients.contains(socket)) {
                BlocksSource::releaseID(clients.take(socket));
                delete socket;
                emit updated();
            }
        } else {
            qCritical() << tr("[TLSServerListener::onClientDeconnection] sender is casted to nullptr T_T");
        }
    } else {
        qDebug() << "[TLSServerListener::onClientDeconnection] sender is nullptr, usually because the connection was closed on our end";
    }
}

void TLSServerListener::onTLSUpdated(bool enabled)
{
    if (enabled)
        actualID = tr("TLS server");
    else
        actualID = tr("TCP server");
}




