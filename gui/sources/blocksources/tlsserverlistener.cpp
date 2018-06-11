#include "tlsserverlistener.h"
#include "shared/sslconf.h"
#include <QDir>
#include <QDebug>
#include "socksproxyhelper.h"

BaseTcpServer::BaseTcpServer(QObject *parent) :
    QTcpServer(parent)
{

}

BaseTcpServer::~BaseTcpServer()
{

}

void BaseTcpServer::incomingConnection(qintptr socketDescriptor)
{
    emit newClient(socketDescriptor);
}


const quint16 TLSServerListener::DEFAULT_PORT = 8000;
const QHostAddress TLSServerListener::DEFAULT_ADDRESS = QHostAddress::LocalHost;
const QString TLSServerListener::ID = QString("TCP/TLS server");

TLSServerListener::TLSServerListener(QHostAddress hostAddress, quint16 hostPort, QObject *parent) :
    IPBlocksSources(hostAddress,hostPort, parent),
    server(nullptr),
    socksProxy(false)
{
    tlsPorts.append(443);
    flags |= REFLEXION_OPTIONS | TLS_OPTIONS | TLS_ENABLED;
    type = SERVER;

    connect(this, &TLSServerListener::sslChanged, this, &TLSServerListener::onTLSUpdated);
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

    updateTimer.moveToThread(&serverThread);
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
        if (i.value() == block->getSourceid()) {
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
            break;
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
        connect(server, &BaseTcpServer::newClient, this, &TLSServerListener::handlingClient,Qt::QueuedConnection);
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
        server->close();
        emit log(tr("stopped %1:%2").arg(hostAddress.toString()).arg(hostPort), actualID, Pip3lineConst::LSTATUS);
        delete server;
        server = nullptr;
        emit stopped();
        triggerUpdate();
    }

    QHashIterator<QSslSocket *, int> i(clients);
    while (i.hasNext()) {
        i.next();
        i.key()->disconnectFromHost();
        delete i.key();
    }

    clients.clear();

    QHashIterator<QSslSocket *, SocksProxyHelper *> j(clientsProxyNeeded);
    while (j.hasNext()) {
        j.next();
        delete j.value();
    }

    clientsProxyNeeded.clear();
}

void TLSServerListener::onConnectionClosed(int cid)
{
    QHashIterator<QSslSocket *, int> i(clients);
    QSslSocket * socket = nullptr;
    while (i.hasNext()) {
        i.next();
        if (i.value() == cid) {
            socket = i.key();
            if (clientsProxyNeeded.contains(socket))
                delete clientsProxyNeeded.take(socket);

            disconnect(socket, &QSslSocket::disconnected, this, &TLSServerListener::onClientDeconnection);
            socket->disconnectFromHost();
            break;
        }
    }

    BlocksSource::releaseID(cid);

    if (socket != nullptr) {
        clients.remove(socket);
        delete socket;
        updateConnectionsInfo();
    }


}

void TLSServerListener::dataReceived()
{
   // qDebug() << "[TLSServerListener::packetReceived]" << sender();

    QSslSocket * socket = dynamic_cast<QSslSocket *>(sender());
    if (socket != nullptr) {

        if (!clients.contains(socket)) { // that's not normal
            qCritical() << tr("Client not found in the recorder list T_T");
            return;
        }
        int clientId = clients.value(socket);
        if (socket->bytesAvailable() > 0) {
            QByteArray data  = socket->readAll();
            if (clientsProxyNeeded.contains(socket)) {
                SocksProxyHelper * proxyHelper = clientsProxyNeeded.value(socket);
                QByteArray resp = proxyHelper->processRequest(data);
                if (proxyHelper->getState() == SocksProxyHelper::DONE) { // all is good
                    // create connection details
                    bool tlscon = tlsPorts.contains(proxyHelper->getPort());
                    ConnectionDetails cd(proxyHelper->getHost(),
                                         proxyHelper->getPort(),
                                         tlscon);
                    // some cleaning first
                    clientsProxyNeeded.remove(socket);
                    delete proxyHelper;
                    // send response to client
                    socket->write(resp);

                    if (tlscon) {
                        if (!startingTLS(socket)) {
                            socket->disconnectFromHost();
                            int cid = clients.take(socket);
                            BlocksSource::releaseID(cid);
                            delete socket;
                            updateConnectionsInfo();
                            return;
                        }
                    }

                    emit newConnectionData(clientId,cd);

                }
                else if (proxyHelper->getState() == SocksProxyHelper::NEED_AUTH_DATA ||
                        proxyHelper->getState() == SocksProxyHelper::AUTHENTICATED) {
                    // just send the SOCKS response if we are still in the middle
                    socket->write(resp);
                    return;
                } else if (proxyHelper->getState() == SocksProxyHelper::REJECTED) {
                    // some part of the process was validated, just send the reponse and close the connection
                    socket->write(resp);
                    socket->disconnectFromHost();
                    socket->close();
                    clientsProxyNeeded.remove(socket);
                    delete proxyHelper;
                    return;
                } else {
                    clientsProxyNeeded.remove(socket);
                    delete proxyHelper;
                    // never mind sending the data received as block
                }
            } else if (isB64Blocks()) {
                processIncomingB64Block(data, clientId);

            } else {
                Block * datab = new(std::nothrow) Block(data, clientId);
                if (datab == nullptr) qFatal("Cannot allocate Block for TLSServerListener X{");

                emit blockReceived(datab);
            }
        }
    } else {
        qCritical() << tr("[TLSServerListener::packetReceived] Casted object is nullptr");
    }
}

void TLSServerListener::handlingClient(qintptr socketDescriptor)
{
    qDebug() << "[TLSServerListener::handlingClient] New client" << socketDescriptor;

    QSslSocket * socket = new(std::nothrow) QSslSocket();
    if (socket == nullptr) {
        qFatal("Cannot allocate memory for QSslSocket X{");
    }

    //connect(socket, qOverload<QAbstractSocket::SocketError>(&QSslSocket::error),this, &TLSServerListener::onError,Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
    connect(socket, &QSslSocket::readyRead, this, &TLSServerListener::dataReceived,Qt::QueuedConnection);

    socket->setSocketDescriptor(socketDescriptor);

    if (socksProxy) {
        SocksProxyHelper * proxyHelper = new(std::nothrow)SocksProxyHelper(hostAddress,hostPort);
        if (proxyHelper == nullptr) {
            qFatal("Cannot allocate memory for SocksProxyHelper X{");
        }

        clientsProxyNeeded.insert(socket, proxyHelper);
    } else if (isTLSEnable()) {
        if (!startingTLS(socket)) {
            emit log(tr("SSL/TLS handshake failed. Closing the connection."),actualID, Pip3lineConst::LERROR);
            socket->disconnectFromHost();
            delete socket;
            socket = nullptr;
        }
    } else {
        qDebug() << tr("TLS is not enabled");
        if (!socket->waitForConnected()) {
            emit log(tr("socket error: %1").arg(socket->errorString()),actualID, Pip3lineConst::LERROR);
            delete socket;
            socket = nullptr;
        }
    }

    if (socket != nullptr) {
        emit log(tr("New %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()),actualID, Pip3lineConst::LSTATUS);
        connect(socket, &QSslSocket::disconnected, this, &TLSServerListener::onClientDeconnection,Qt::QueuedConnection);

        clients.insert(socket, BlocksSource::newSourceID(this));
        updateConnectionsInfo();
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

    QSslSocket * sobject = dynamic_cast<QSslSocket *>(sender());
    if (error == QAbstractSocket::RemoteHostClosedError) {
        if (sobject != nullptr) {
            handlingDisconnect(sobject);
        } else {
            qDebug() << "[TLSServerListener::onError] *a* connection was closed by the remote client";
        }
    }  else if (error == QAbstractSocket::SslHandshakeFailedError) {
        qDebug() << "[TLSServerListener::onError] SSL/TLS handshake failed";
        if (sobject != nullptr) {
            handlingDisconnect(sobject);
        }
    } else {
        qDebug() << "[TLSServerListener::onError]" << error;
    }
}

void TLSServerListener::onSSLModeChange(QSslSocket::SslMode mode)
{
    QSslSocket * socket = dynamic_cast<QSslSocket *>(sender());
    if (socket != nullptr) {
        if (clients.contains(socket))
          qDebug() << tr("[TLSServerListener] SSL Mode changed  %1 => %3").arg(clients.value(socket)).arg(SslConf::sslModeToString(mode));
        else
          qCritical() << tr("[TLSServerListener::onSSLModeChange] socket not found T_T");
    } else {
        qDebug() << "[TLSServerListener::onSSLModeChange] null object, mode:" << mode;
    }
}

void TLSServerListener::onClientDeconnection()
{
    QObject *obj = sender();
    if (obj != nullptr) {
        QSslSocket * socket = dynamic_cast<QSslSocket *>(obj);
        if (socket != nullptr) {
            handlingDisconnect(socket);
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

void TLSServerListener::internalUpdateConnectionsInfo()
{
    connectionsInfo.clear();
    QHashIterator<QSslSocket *, int> i(clients);
    while (i.hasNext()) {
        i.next();
        QSslSocket * socket = i.key();
        QString desc = socket->peerAddress().toString();
        if (isTLSEnable())
            desc.append(QString(":%1/TLS").arg(socket->peerPort()));
        else
            desc.append(QString(":%1/TCP").arg(socket->peerPort()));
        Target<BlocksSource *> tac;
        tac.setDescription(desc);
        tac.setConnectionID(i.value());
        tac.setSource(this);
        connectionsInfo.append(tac);
    }
}

bool TLSServerListener::startingTLS(QSslSocket *sslsocket)
{
    if (sslConfiguration->getSslConfiguration().localCertificate().isNull()) {
        emit log(tr("No server certificate loaded, failing the SSL/TLS connection"), actualID, Pip3lineConst::LERROR);
        return false;
    }

    if (sslConfiguration->getSslConfiguration().privateKey().isNull()) {
        emit log(tr("No private key loaded, failing the SSL/TLS connection"), actualID, Pip3lineConst::LERROR);
        return false;
    }

    qDebug() << tr("Trying to start TLS for %1:%2").arg(sslsocket->peerAddress().toString()).arg(sslsocket->peerPort());
    //connect(sslsocket, qOverload<const QList<QSslError> &>(&QSslSocket::sslErrors), this, &TLSServerListener::onSslErrors,Qt::QueuedConnection);
    connect(sslsocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)), Qt::QueuedConnection);
    connect(sslsocket, &QSslSocket::modeChanged,this, &TLSServerListener::onSSLModeChange,Qt::QueuedConnection);
    sslsocket->setSslConfiguration(sslConfiguration->getSslConfiguration());
    sslsocket->startServerEncryption();

    return true;
}

void TLSServerListener::handlingDisconnect(QSslSocket *socket)
{
    emit log(tr("Disconnection for %1:%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()), actualID, Pip3lineConst::LSTATUS);
    if (clients.contains(socket)) {
        int rsid = clients.take(socket);

        // clearing any remaining b64 data for this SID
        if (b64BlockTempDataList.contains(rsid)) {
            b64DecodeAndEmit(b64BlockTempDataList.take(rsid), rsid);
        }
        BlocksSource::releaseID(rsid);

        delete socket;
        emit connectionClosed(rsid);
        updateConnectionsInfo();
    }
}

bool TLSServerListener::isSocks5Proxy() const
{
    return socksProxy;
}

void TLSServerListener::setSocks5Proxy(bool value)
{
    socksProxy = value;
}




