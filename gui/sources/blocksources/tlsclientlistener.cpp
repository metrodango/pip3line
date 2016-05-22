#include "connectionswidget.h"
#include "tlsclientlistener.h"
#include "shared/sslconf.h"
#include <QTabWidget>

const quint16 TLSClientListener::DEFAULT_PORT = 443;
const QHostAddress TLSClientListener::DEFAULT_ADDRESS = QHostAddress("216.21.170.226");
const QString TLSClientListener::ID = QString("TCP/TLS client");

TLSClientListener::TLSClientListener(QHostAddress hostAddress, quint16 hostPort, QObject *parent) :
    IPBlocksSources(hostAddress, hostPort, parent),
    running(false)
{
    flags |= REFLEXION_OPTIONS | TLS_OPTIONS | TLS_ENABLED;
    type = CLIENT;
    connect(this, SIGNAL(sslChanged(bool)), this, SLOT(onTLSUpdated(bool)));

    setTlsEnable(true);

    moveToThread(&workerThread);
    workerThread.start();
}

TLSClientListener::~TLSClientListener()
{
    workerThread.quit();
    workerThread.wait();
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

void TLSClientListener::sendBlock(Block *block)
{
    if (running) {
        QByteArray data = applyOutboundTransform(block->getData());
        qint64 size = data.size();
        qint64 bwritten = 0;

        int sid = -1;
        if (mapExtSourcesToLocal.contains(block->getSourceid())) {
            sid = mapExtSourcesToLocal.value(block->getSourceid());
        }
        int bid = block->getSourceid();
        bool foundSource = false;

        QHashIterator<QSslSocket *, int> i(sockets);
        while (i.hasNext()) {
            i.next();
            int suid = i.value();
            QSslSocket * socket = i.key();
            if (bid ==  suid || sid == suid) { // either this we are sending directly to the blocksource or getting the block from another one
                bwritten = socket->write(data);
                while (size > bwritten) {
                    bwritten += socket->write(&(data.data()[bwritten - 1]),size - bwritten);
                }
                foundSource = true;
            }
        }

        if (!foundSource) { // or we open a new connection
            sid = BlocksSource::newSourceID(this);

            QSslSocket * socket = nullptr;

            socket = new(std::nothrow) QSslSocket();
            if (socket == nullptr) {
                qFatal("Cannot allocate memory for QSslSocket X{");
            }
            connect(socket, SIGNAL(readyRead()), this, SLOT(dataReceived()), Qt::QueuedConnection);
            connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(onError(QAbstractSocket::SocketError)),Qt::QueuedConnection);
            connect(socket, SIGNAL(disconnected()), this, SLOT(onClientDeconnection()),Qt::QueuedConnection);
            sockets.insert(socket,sid);

            if (isTLSEnable()) {
                qDebug() << "CA certs:" << sslConfiguration->getSslConfiguration().caCertificates().size();
                connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(onSslErrors(QList<QSslError>)),Qt::QueuedConnection);
                connect(socket, SIGNAL(modeChanged(QSslSocket::SslMode)),this, SLOT(onSSLModeChange(QSslSocket::SslMode)),Qt::QueuedConnection);
                connect(socket, SIGNAL(encrypted()), this, SLOT(onTLSStarted()));
                socket->setSslConfiguration(sslConfiguration->getSslConfiguration());

                socket->connectToHostEncrypted(hostAddress.toString(), hostPort,sslConfiguration->getSslPeerNameSNI());
            } else {
                connect(socket , SIGNAL(connected()), this, SLOT(onPlainStarted()));
                socket->connectToHost(hostAddress, hostPort);
            }

            if (sockets.size() > 1000000)
                emit log(tr("The number of client \"connections\" as reached 1 Millions. Dude for real ?"),actualID, Pip3lineConst::LERROR);

            if (bid != Block::INVALID_ID) // only adding when the block SID is valid
                mapExtSourcesToLocal.insert(bid, sid);

            bwritten = socket->write(data);
            while (size > bwritten) {
                bwritten += socket->write(&(data.data()[bwritten - 1]),size - bwritten);
            }

            emit updated();
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
        emit updated();
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
        emit stopped();
        emit updated();
    }
}

QList<Target<BlocksSource *> > TLSClientListener::getAvailableConnections()
{
    QList<Target<BlocksSource *> > list;

    if (running) { // accepting new connections
        Target<BlocksSource *> tac;
        tac.setConnectionID(Block::INVALID_ID);
        QString desc;
        desc = desc.append(QString("[%1] %2:%3/%4")
                .arg(BlocksSource::NEW_CONNECTION_STRING)
                .arg(hostAddress.toString())
                .arg(hostPort))
                .arg(isTLSEnable() ? QString("TLS") : QString("TCP"));
        tac.setDescription(desc);
        tac.setSource(this);
        list.append(tac);

        QHashIterator<QSslSocket *, int> i(sockets);
        while (i.hasNext()) {
            i.next();
            QString desc;
            desc = desc.append(QString("[%1] %2:%3/%4")
                    .arg(i.value())
                    .arg(hostAddress.toString())
                    .arg(hostPort))
                    .arg(isTLSEnable() ? tr("TLS") : tr("TCP"));
            Target<BlocksSource *> tac;
            tac.setDescription(desc);
            tac.setConnectionID(i.value());
            tac.setSource(this);
            list.append(tac);
        }
    }

    return list;
}

void TLSClientListener::dataReceived()
{
    QSslSocket * socket = static_cast<QSslSocket *>(sender());
    if (socket != nullptr) {
        if (!sockets.contains(socket)) {
            qCritical() << tr("[TLSClientListener::dataReceived] Unknown client T_T");
            return;
        }

        if (socket->bytesAvailable() > 0) {
            QByteArray data;

            data = socket->readAll();

            if (data.isEmpty()){
                qCritical() << tr("Received data block is empty, ignoring. T_T");
                return;
            }

            int sid = sockets.value(socket);
            Block * datab = new(std::nothrow) Block(data,sid);
            if (datab == nullptr) qFatal("Cannot allocate Block for TLSClientListener X{");

            QHashIterator<int, int> i(mapExtSourcesToLocal);
            while (i.hasNext()) {
                i.next();
                if (i.value() == sid)
                    datab->setSourceid(i.key());
            }

            emit blockReceived(datab);
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
        QSslSocket * sobject = static_cast<QSslSocket *>(sender());
        if (sobject != nullptr) {
            emit log(sobject->errorString(), actualID, Pip3lineConst::LERROR);
        } else {
            qDebug() << "[TLSClientListener::onError] a connection was closed by the remote client";
        }
    }
}

void TLSClientListener::onSSLModeChange(QSslSocket::SslMode mode)
{
    QSslSocket * socket = static_cast<QSslSocket *>(sender());
    if (socket != nullptr) {
        qDebug() << "SSL Mode changed for " << socket->peerAddress() << "/" << socket->peerPort() << mode;
    } else {
        qDebug() << "[TLSClientListener::onPeerVerificationError] null casting, mode:" << mode;
    }
}

void TLSClientListener::onClientDeconnection()
{
    QObject *obj = sender();
    if (obj != nullptr) {
        QSslSocket * socket = static_cast<QSslSocket *>(obj);
        if (socket != nullptr) {
            emit log(tr("Disconnection for %1/%2").arg(socket->peerAddress().toString()).arg(socket->peerPort()), actualID, Pip3lineConst::LSTATUS);
            if (sockets.contains(socket)) {
                BlocksSource::releaseID(sockets.take(socket));
                delete socket;
                emit updated();
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
    QSslSocket * socket = static_cast<QSslSocket *>(sender());
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
#if QT_VERSION >= 0x050000

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
#else
    QString info = sobject->peerCertificate().subjectInfo(QSslCertificate::CommonName);

    if (!info.isEmpty()) {
        mess.append("CN: ").append(info);
    }

    info = sobject->peerCertificate().subjectInfo(QSslCertificate::Organization);

    if (!info.isEmpty()) {
        mess.append(" O: ").append(info);
    }
    info = sobject->peerCertificate().subjectInfo(QSslCertificate::OrganizationalUnitName);
    if (!info.isEmpty()) {
        mess.append(" OU: ").append(info);
    }

    info = sobject->peerCertificate().subjectInfo(QSslCertificate::CountryName);
    if (!info.isEmpty()) {
        mess.append(" Country: ").append(info);
    }

#endif
    mess.append("]");
    emit log(mess, actualID, Pip3lineConst::LSTATUS);
}

void TLSClientListener::onPlainStarted()
{
    QSslSocket * socket = static_cast<QSslSocket *>(sender());
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
    emit log(mess, actualID, Pip3lineConst::LSTATUS);
}

void TLSClientListener::onTLSUpdated(bool enabled)
{
    if (enabled)
        actualID = tr("TLS client");
    else
        actualID = tr("TCP client");
}

