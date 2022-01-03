#include "connectionswidget.h"
#include "tlsclientlistener.h"
#include "shared/sslconf.h"
#include "shared/newconnectionbutton.h"
#include <QTabWidget>
#include <QPushButton>
#include <QDebug>

const quint16 TLSClientListener::DEFAULT_PORT = 443;
const QHostAddress TLSClientListener::DEFAULT_ADDRESS = QHostAddress("127.0.0.1");
const QString TLSClientListener::ID = QString("TCP/TLS client");

TLSClientListener::TLSClientListener(QHostAddress nhostAddress, quint16 nhostPort, QObject *parent) :
    IPBlocksSources(nhostAddress, nhostPort, parent),
    running(false)
{
    flags = REFLEXION_OPTIONS | TLS_OPTIONS | TLS_ENABLED | GEN_IP_OPTIONS | B64BLOCKS_OPTIONS;

    type = CLIENT;
    connect(this, &TLSClientListener::sslChanged, this, &TLSClientListener::onTLSUpdated);

    setTlsEnable(true);

    updateConnectionsTimer.moveToThread(&workerThread);
    moveToThread(&workerThread);
    workerThread.start();
}

TLSClientListener::~TLSClientListener()
{
    workerThread.quit();
    workerThread.wait();
    mapExtSourcesToLocal.clear();
}

QString TLSClientListener::getName()
{
    return actualID;
}

QString TLSClientListener::getDescription()
{
    return tr("Simple TCP/TLS client listener");
}

bool TLSClientListener::isStarted()
{
    return running;
}

QWidget *TLSClientListener::getAdditionnalCtrls(QWidget *parent)
{
    QPushButton * but = new(std::nothrow)NewConnectionButton (parent);
    if (but == nullptr) {
        qFatal("Cannot allocate memory for QPushButton X{");
    }
    connect(but, &QPushButton::clicked, this, &TLSClientListener::createConnection, Qt::QueuedConnection);
    return but;
}

int TLSClientListener::getTargetIdFor(int sourceId)
{
    int targetId = Block::INVALID_ID;
    if (sourceId != Block::INVALID_ID) {
        int mappedSid = Block::INVALID_ID;
        if (mapExtSourcesToLocal.contains(sourceId)) {
            mappedSid = mapExtSourcesToLocal.value(sourceId);
        }

        foreach (int suid , sockets) {
            // either we are sending directly to the blocksource or getting the block from another one
            if (sourceId == suid || mappedSid == suid) {
                targetId = suid;
                break;
            }
        }
    }
    return targetId;
}

void TLSClientListener::sendBlock(Block *block)
{
    if (running) {
       // qDebug() << "Sending block";
        QByteArray data = applyOutboundTransform(block->getData());
        qint64 size = data.size();
        qint64 bwritten = 0;

        int sid = Block::INVALID_ID;
        int bid = block->getSourceid();
        if (mapExtSourcesToLocal.contains(bid)) {
            sid = mapExtSourcesToLocal.value(bid);
        }

        bool foundSource = false;

        QHashIterator<QSslSocket *, int> i(sockets);
        while (i.hasNext()) {
            i.next();
            int suid = i.value();
            QSslSocket * socket = i.key();
            if (bid ==  suid || sid == suid) { // either we are sending directly to the blocksource or getting the block from another one
                bwritten = socket->write(data);
                while (size > bwritten) {
                    bwritten += socket->write(&(data.data()[bwritten - 1]),size - bwritten);
                }
                foundSource = true;
                break;
            }
        }

        if (!foundSource) { // or we open a new connection
            sid = BlocksSource::newSourceID(this);

            QSslSocket * socket = new(std::nothrow) QSslSocket();
            if (socket == nullptr) {
                qFatal("Cannot allocate memory for QSslSocket X{");
            }
            connect(socket, &QSslSocket::readyRead, this, &TLSClientListener::dataReceived, Qt::QueuedConnection);
            //connect(socket, qOverload<QAbstractSocket::SocketError>(&QSslSocket::error),this, &TLSClientListener::onError,Qt::QueuedConnection);
            connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
            connect(socket, &QSslSocket::disconnected, this, &TLSClientListener::onClientDeconnection,Qt::QueuedConnection);

            sockets.insert(socket,sid);

            QHostAddress tempAddr = hostAddress;
            quint16 tempPort = hostPort;
            bool tlsEnabled = isTLSEnabled();

            if (specificConnectionsData.contains(bid)) {
                ConnectionDetails cd = specificConnectionsData.value(bid);
                tempAddr = cd.getAddress();
                tempPort = cd.getPort();
                tlsEnabled = cd.isTlsEnabled();
                specificConnectionsData.remove(bid);
                qDebug() << "Socks Client" << tempAddr.toString() << tempPort << (tlsEnabled ? QString("true") : QString("False"));
            }

            if (tlsEnabled) {
                qDebug() << "CA certs:" << sslConfiguration->getSslConfiguration().caCertificates().size();
                //connect(socket, qOverload<const QList<QSslError> &>(&QSslSocket::sslErrors), this, &TLSClientListener::onSslErrors,Qt::QueuedConnection);
                connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)), Qt::QueuedConnection);
                connect(socket, &QSslSocket::modeChanged,this, &TLSClientListener::onSSLModeChange,Qt::QueuedConnection);
                connect(socket, &QSslSocket::encrypted, this, &TLSClientListener::onTLSStarted);
                socket->setSslConfiguration(sslConfiguration->getSslConfiguration());
                socket->connectToHostEncrypted(tempAddr.toString(), tempPort,sslConfiguration->getSslPeerNameSNI());
            } else {
                connect(socket , &QSslSocket::connected, this, &TLSClientListener::onPlainStarted);
                socket->connectToHost(tempAddr, tempPort);
            }

            if (sockets.size() > 10000)
                emit log(tr("The number of client \"connections\" as reached 10 000. Dude for real ?"),actualID, Pip3lineConst::LERROR);

            if (bid != Block::INVALID_ID) // only adding when the block SID is valid
                mapExtSourcesToLocal.insert(bid, sid);

            bwritten = socket->write(data);
            while (size > bwritten) {
                bwritten += socket->write(&(data.data()[bwritten - 1]),size - bwritten);
            }

            updateConnectionsInfo();
        }
    } else {
        emit log(tr("The client is not started, cannot send the packet"),actualID,Pip3lineConst::LERROR);
    }

    delete block;
}

bool TLSClientListener::startListening()
{
    if (!running) {
        // really basic, a client listener never fails to "start"
        running = true;
        emit started();
        updateConnectionsInfo();
    }

    return true;
}

void TLSClientListener::stopListening()
{
    if (running) {
        running = false;

        QHashIterator<QSslSocket *, int> i(sockets);
        while (i.hasNext()) {
            i.next();
            BlocksSource::releaseID(i.value());
            delete i.key();
        }

        sockets.clear();
        mapExtSourcesToLocal.clear();
        triggerUpdate();
        emit stopped();
    }
}

void TLSClientListener::setSpecificConnection(int sourceId, ConnectionDetails cd)
{
    specificConnectionsData.insert(sourceId, cd);
}

void TLSClientListener::onConnectionClosed(int cid)
{
    qDebug() << "[TLSClientListener::onConnectionClosed] received" << cid;
    // cleaning any connection details
    if (specificConnectionsData.contains(cid))
        specificConnectionsData.remove(cid);

    // checking mappings
    if (mapExtSourcesToLocal.contains(cid)) {
        cid = mapExtSourcesToLocal.take(cid);
    }

    QSslSocket * socket = nullptr;
    QHashIterator<QSslSocket *, int> i(sockets);
    while (i.hasNext()) {
        i.next();
        if (i.value() == cid) {
            socket = i.key();
            disconnect(socket, &QSslSocket::disconnected, this, &TLSClientListener::onClientDeconnection);
            socket->disconnectFromHost();
            delete socket;
            break;
        }
    }

    if (socket != nullptr) {
        sockets.remove(socket);

        BlocksSource::releaseID(cid);
        updateConnectionsInfo();
    } else {
        qDebug() << "[TLSClientListener::onConnectionClosed] Socket not found" << cid;
    }
}

void TLSClientListener::createConnection()
{
    sid = BlocksSource::newSourceID(this);

    QSslSocket * socket = new(std::nothrow) QSslSocket();
    if (socket == nullptr) {
        qFatal("Cannot allocate memory for QSslSocket X{");
    }
    connect(socket, &QSslSocket::readyRead, this, &TLSClientListener::dataReceived, Qt::QueuedConnection);
    // connect(socket, qOverload<QAbstractSocket::SocketError>(&QSslSocket::error),this, &TLSClientListener::onError,Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)), Qt::QueuedConnection);
    connect(socket, &QSslSocket::disconnected, this, &TLSClientListener::onClientDeconnection,Qt::QueuedConnection);

    sockets.insert(socket,sid);

    QHostAddress tempAddr = hostAddress;
    quint16 tempPort = hostPort;
    bool tlsEnabled = isTLSEnabled();

    if (tlsEnabled) {
        qDebug() << "CA certs:" << sslConfiguration->getSslConfiguration().caCertificates().size();
        // connect(socket, qOverload<const QList<QSslError> &>(&QSslSocket::sslErrors), this, &TLSClientListener::onSslErrors,Qt::QueuedConnection);
        connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)), Qt::QueuedConnection);
        connect(socket, &QSslSocket::modeChanged,this, &TLSClientListener::onSSLModeChange,Qt::QueuedConnection);
        connect(socket, &QSslSocket::encrypted, this, &TLSClientListener::onTLSStarted);
        socket->setSslConfiguration(sslConfiguration->getSslConfiguration());
        socket->connectToHostEncrypted(tempAddr.toString(), tempPort,sslConfiguration->getSslPeerNameSNI());
    } else {
        connect(socket , &QSslSocket::connected, this, &TLSClientListener::onPlainStarted);
        socket->connectToHost(tempAddr, tempPort);
    }

    if (sockets.size() > 10000)
        emit log(tr("The number of client \"connections\" as reached 10 000. Dude for real ?"),actualID, Pip3lineConst::LERROR);

    updateConnectionsInfo();
}

void TLSClientListener::dataReceived()
{
    QSslSocket * socket = dynamic_cast<QSslSocket *>(sender());
    if (socket != nullptr) {
        if (!sockets.contains(socket)) {
            qCritical() << tr("[TLSClientListener::dataReceived] Unknown client T_T");
            return;
        }
        int sid = sockets.value(socket);

        if (socket->bytesAvailable() > 0) {
            QByteArray data;

            data = socket->readAll();

            if (data.isEmpty()){
                qCritical() << tr("Received data block is empty, ignoring. T_T");
                return;
            }

            QHashIterator<int, int> i(mapExtSourcesToLocal);
            while (i.hasNext()) {
                i.next();
                if (i.value() == sid) {
                    sid = i.key();
                    break;
                }
            }

            data = applyInboundTransform(data);
            if (!data.isEmpty()) {
                Block * datab = new(std::nothrow) Block(data,sid);
                if (datab == nullptr) qFatal("Cannot allocate Block for TLSClientListener X{");

                emit blockReceived(datab);
            }
        }
    } else {
        qCritical() << tr("[TLSClientListener::dataReceived] cast failed");
    }

}

void TLSClientListener::onSslErrors(const QList<QSslError> &errors)
{
    for (int i = 0; i < errors.size(); i++) {
        emit log(errors.at(i).errorString(), actualID, Pip3lineConst::LWARNING);
    }
}

void TLSClientListener::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "[TLSClientListener::onError]" << error;
    if (error != QAbstractSocket::RemoteHostClosedError) {
        QSslSocket * sobject = dynamic_cast<QSslSocket *>(sender());
        if (sobject != nullptr) {
            emit log(sobject->errorString(), actualID, Pip3lineConst::LERROR);
        } else {
            qDebug() << "[TLSClientListener::onError] a connection was closed by the remote client";
        }
    }
}

void TLSClientListener::onSSLModeChange(QSslSocket::SslMode mode)
{
    QSslSocket * socket = dynamic_cast<QSslSocket *>(sender());
    if (socket != nullptr) {
        if (sockets.contains(socket))
          qDebug() << tr("[TLSClientListener] SSL Mode changed  %1 => %3").arg(sockets.value(socket)).arg(SslConf::sslModeToString(mode));
        else
          qCritical() << tr("[TLSClientListener::onSSLModeChange] socket not found T_T");
    } else {
        qDebug() << "[TLSClientListener::onSSLModeChange] null casting, mode:" << mode;
    }
}

void TLSClientListener::onClientDeconnection()
{
    QObject *obj = sender();
    if (obj != nullptr) {
        QSslSocket * socket = dynamic_cast<QSslSocket *>(obj);
        if (socket != nullptr) {
            emit log(tr("Disconnection for %1/%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()), actualID, Pip3lineConst::PLSTATUS);
            if (sockets.contains(socket)) {
                int cid = sockets.take(socket);

                int mid = Block::INVALID_ID;
                QHashIterator<int, int> i(mapExtSourcesToLocal);
                ClosingSocketWorker * cw = new(std::nothrow) ClosingSocketWorker();
                connect(cw, &ClosingSocketWorker::connectionClosed, this, &TLSClientListener::connectionClosed,Qt::QueuedConnection);
                while (i.hasNext()) {
                    i.next();
                    if (i.value() == cid) {
                        mid = i.key();
                        cw->add(mid);
                        break;
                    }
                }

                QTimer::singleShot(300, cw, SLOT(sendEverything()));

                // cleaning any mappings
                if (mid != Block::INVALID_ID)
                    mapExtSourcesToLocal.remove(mid);

                // cleaning any connection details
                if (specificConnectionsData.contains(cid))
                    specificConnectionsData.remove(cid);

                BlocksSource::releaseID(cid);
                delete socket;
                updateConnectionsInfo();
            }
        } else {
            qCritical() << "[TLSClientListener::onClientDeconnection] sender is casted to nullptr";
        }
    } else {
        qDebug() << "[TLSClientListener::onClientDeconnection] sender is nullptr, usually because the connection was closed on our end";
    }
}

void TLSClientListener::onTLSStarted()
{
    QSslSocket * socket = dynamic_cast<QSslSocket *>(sender());
    if (socket == nullptr) {
        qCritical() << tr("[TLSClientListener::onTLSStarted] Casted object is nullptr");
        return;
    }
    QString mess;
    if (sslConfiguration->getSslPeerNameSNI().isEmpty())
        mess.append(tr("Connected to %1:%2 ").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
    else
        mess.append(tr("Connected to %1(%2):%3 ")
                    .arg(sslConfiguration->getSslPeerNameSNI())
                    .arg(socket->peerAddress().toString())
                    .arg(socket->peerPort()));

    QStringList infoList = socket->peerCertificate().subjectInfo(QSslCertificate::CommonName);

    if (!infoList.isEmpty()) {
        mess.append("[CN: ");
        for (int i = 0; i < infoList.size(); i++)
            mess.append(QString("%1,").arg(infoList.at(i)));
        mess.chop(1);
    }

    infoList = socket->peerCertificate().subjectInfo(QSslCertificate::Organization);

    if (!infoList.isEmpty()) {
        mess.append(" O: ");
        for (int i = 0; i < infoList.size(); i++)
            mess.append(QString("%1,").arg(infoList.at(i)));
        mess.chop(1);
    }
    infoList = socket->peerCertificate().subjectInfo(QSslCertificate::OrganizationalUnitName);
    if (!infoList.isEmpty()) {
        mess.append(" OU: ");
        for (int i = 0; i < infoList.size(); i++)
            mess.append(QString("%1,").arg(infoList.at(i)));
        mess.chop(1);
    }

    infoList = socket->peerCertificate().subjectInfo(QSslCertificate::CountryName);
    if (!infoList.isEmpty()) {
        mess.append(" Country: ");
        for (int i = 0; i < infoList.size(); i++)
            mess.append(QString("%1,").arg(infoList.at(i)));
        mess.chop(1);
    }
    mess.append("]");
    emit log(mess, actualID, Pip3lineConst::PLSTATUS);
}

void TLSClientListener::onPlainStarted()
{
    QSslSocket * socket = dynamic_cast<QSslSocket *>(sender());
    if (socket == nullptr) {
        qCritical() << tr("[TLSClientListener::onPlainStarted] Casted object is nullptr");
        return;
    }
    QString mess;
    if (sslConfiguration->getSslPeerNameSNI().isEmpty())
        mess.append(tr("Connected to %1:%2 ").arg(socket->peerAddress().toString()).arg(socket->peerPort()));
    else
        mess.append(tr("Connected to %1(%2):%3 ")
                    .arg(sslConfiguration->getSslPeerNameSNI())
                    .arg(socket->peerAddress().toString())
                    .arg(socket->peerPort()));
    emit log(mess, actualID, Pip3lineConst::PLSTATUS);
}

void TLSClientListener::onTLSUpdated(bool enabled)
{
    if (enabled)
        actualID = tr("TLS client");
    else
        actualID = tr("TCP client");
}

void TLSClientListener::internalUpdateConnectionsInfo()
{
    connectionsInfo.clear();

    if (running) { // accepting new connections
        Target<BlocksSource *> tac;
        tac.setConnectionID(Block::INVALID_ID);
        QString desc;
        desc = desc.append(QString("[%1] %2:%3/%4")
                .arg(BlocksSource::NEW_CONNECTION_STRING)
                .arg(hostAddress.toString())
                .arg(hostPort))
                .arg(isTLSEnabled() ? QString("TLS") : QString("TCP"));
        tac.setDescription(desc);
        tac.setSource(this);
        connectionsInfo.append(tac);

        QHashIterator<QSslSocket *, int> i(sockets);
        while (i.hasNext()) {
            i.next();
            QString desc;
            desc = desc.append(QString("[%1] %2:%3/%4")
                    .arg(i.value())
                    .arg(hostAddress.toString())
                    .arg(hostPort))
                    .arg(isTLSEnabled() ? tr("TLS") : tr("TCP"));
            Target<BlocksSource *> tac;
            tac.setDescription(desc);
            tac.setConnectionID(i.value());
            tac.setSource(this);
            connectionsInfo.append(tac);
        }
    }
}


ClosingSocketWorker::ClosingSocketWorker(QObject *parent) :
    QObject(parent)
{

}

void ClosingSocketWorker::add(int cid)
{
    list.append(cid);
}

void ClosingSocketWorker::sendEverything()
{
    for (int i = 0; i < list.size(); i++) {
        emit connectionClosed(list.at(i));
    }

    deleteLater();
}
