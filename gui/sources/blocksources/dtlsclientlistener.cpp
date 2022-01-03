#include "dtlsclientlistener.h"
#include "shared/sslconf.h"
#include "shared/newconnectionbutton.h"
#include "connectionswidget.h"
#include "dtlscommon.h"
#include "baseblocksourcewidget.h"
#include "dtlsclientlistenerwidget.h"
#include <QPushButton>
#include <QDebug>
#include <QSslPreSharedKeyAuthenticator>

const int DTLSClientListener::MAX_DTLS_DATAGRAM_SIZE_HARD = 64000;
const quint16 DTLSClientListener::DEFAULT_PORT = 443;
const QHostAddress DTLSClientListener::DEFAULT_ADDRESS = QHostAddress("127.0.0.1");
const QString DTLSClientListener::ID = QString("DTLS client");
const QString DTLSClientListener::USE_SRC_PORT = QString("use_src_port");

DTLSClientListener::DTLSClientListener(QHostAddress nhostAddress, quint16 nhostPort, QObject *parent) :
    IPBlocksSources(nhostAddress, nhostPort, parent),
    running(false),
    useSrcPorts(false)
{
    flags = REFLEXION_OPTIONS | TLS_OPTIONS | TLS_ENABLED | GEN_IP_OPTIONS;

    type = CLIENT;
    setTlsEnable(true);

    QSslConfiguration config = QSslConfiguration::defaultDtlsConfiguration();
    config.setDtlsCookieVerificationEnabled(true);
    config.setPeerVerifyMode(QSslSocket::VerifyNone);
    config.setProtocol(QSsl::DtlsV1_0OrLater);
    sslConfiguration->setSslConfiguration(config);

    updateConnectionsTimer.moveToThread(&workerThread);
    moveToThread(&workerThread);
    workerThread.start();
}

DTLSClientListener::~DTLSClientListener()
{
    workerThread.quit();
    workerThread.wait();
    mapExtSourcesToLocal.clear();
}

QString DTLSClientListener::getName()
{
    return ID;
}

QString DTLSClientListener::getDescription()
{
    return ID;
}

bool DTLSClientListener::isStarted()
{
    return running;
}

QWidget *DTLSClientListener::getAdditionnalCtrls(QWidget *parent)
{
    QPushButton * but = new(std::nothrow)NewConnectionButton (parent);
    if (but == nullptr) {
        qFatal("Cannot allocate memory for QPushButton X{");
    }
    connect(but, &QPushButton::clicked, this, &DTLSClientListener::createConnection, Qt::QueuedConnection);
    return but;
}

int DTLSClientListener::getTargetIdFor(int sourceId)
{
    int targetId = Block::INVALID_ID;
    if (sourceId != Block::INVALID_ID) {
        int mappedSid = Block::INVALID_ID;
        if (mapExtSourcesToLocal.contains(sourceId)) {
            mappedSid = mapExtSourcesToLocal.value(sourceId);
        }
        QHashIterator<QUdpSocket *, QSharedPointer<ConnectionDetails>> i(sockets);
        while (i.hasNext()) {
            i.next();
            QSharedPointer<ConnectionDetails> cd = i.value();
            int suid = cd->getSid();
            // either we are sending directly to the blocksource or getting the block from another one
            if (sourceId == suid || mappedSid == suid) {
                targetId = suid;
                break;
            }
        }
    }
    return targetId;
}

void DTLSClientListener::sendBlock(Block *block)
{
    if (running) {
        //qDebug() << "sending block";
        QByteArray data = applyOutboundTransform(block->getData());
        if (data.size() > MAX_DTLS_DATAGRAM_SIZE_HARD ) {
            emit log(tr("The DTLS packet is too large. It will be truncated to %1 bytes").arg(MAX_DTLS_DATAGRAM_SIZE_HARD),ID,Pip3lineConst::LWARNING);
            data  = data.mid(0,MAX_DTLS_DATAGRAM_SIZE_HARD);
        }

        int bid = block->getSourceid();
        int sid = mapExtSourcesToLocal.value(bid, Block::INVALID_ID);

        bool foundSource = false;
        //qDebug() << "block" << data.size();
        QHashIterator<QUdpSocket *, QSharedPointer<ConnectionDetails>> i(sockets);
        while (i.hasNext()) {
            i.next();
            QSharedPointer<ConnectionDetails> cd = i.value();
            int suid = cd->getSid();
            if (bid == suid || sid == suid) { // either we are sending directly to the blocksource or getting the block from another one
                if (!block->isMultiplexed() || block->getProtocol() == Pip3lineCommon::dtls_protocol) {
                    QSharedPointer<QDtls> crypto = cd->getDtlscontext();
                    if (!crypto.isNull()) {
                        if (crypto->isConnectionEncrypted()) {
                            qDebug() << "sending packet";
                            if (crypto->writeDatagramEncrypted(i.key(), data) != data.size()) {
                                emit log(tr("The DTLS packet was not send entirely '-_-: %1").arg(crypto->dtlsErrorString()),ID,Pip3lineConst::LWARNING);
                            }
                        } else {
                            emit log(tr("[DTLSClientListener::sendBlock] the DTLS handshake has not been completed, ignoring send"),ID,Pip3lineConst::LERROR);
                        }
                    } else {
                        emit log(tr("[DTLSClientListener::sendBlock] Cannot find the DTLS details T_T"),ID,Pip3lineConst::LERROR);
                    }
                } else {
                    qint64 bwritten = i.key()->writeDatagram(data,hostAddress,hostPort);
                    if (bwritten != data.size()) {
                        emit log(tr("The UDP packet was not send entirely '-_-"), ID, Pip3lineConst::LWARNING);
                    }
                }
                foundSource = true;
                break;
            }
        }

        if (!foundSource) { // or we open a new connection
            //qDebug() << "new connection";

            QUdpSocket * socket = new(std::nothrow) QUdpSocket();
            if (socket == nullptr) {
                qFatal("Cannot allocate memory for QUdpSocket X{");
            }
            connect(socket, &QUdpSocket::readyRead, this, &DTLSClientListener::dataReceived);

            if (!socket->bind(QHostAddress::Any, (useSrcPorts ? block->getSrcPort() : 0))) {
                emit log(tr("Binding socket error: %1 (packet dropped)").arg(socket->errorString()), ID, Pip3lineConst::LERROR);
                delete socket;
                delete block;
                return;
            }

            sid = BlocksSource::newSourceID(this);
            QSharedPointer<ConnectionDetails> uc(new(std::nothrow) ConnectionDetails(hostAddress, hostPort));
            uc->setSid(sid);
            sockets.insert(socket,uc);
            mapExtSourcesToLocal.insert(bid, sid);

            if (!block->isMultiplexed() || block->getProtocol() == Pip3lineCommon::dtls_protocol) {

                QSharedPointer<QDtls> crypto = QSharedPointer<QDtls>(new(std::nothrow) QDtls(QSslSocket::SslClientMode));

                crypto->setPeer(hostAddress, hostPort, sslConfiguration->getSslPeerNameSNI());
                crypto->setMtuHint(1200);
                QSslConfiguration sslconf = sslConfiguration->getSslConfiguration();
                qDebug() << "ssl verify" << sslconf.peerVerifyMode();
                crypto->setDtlsConfiguration(sslconf);

                connect(crypto.data(), &QDtls::handshakeTimeout, this, &DTLSClientListener::handshakeTimeout);
                connect(crypto.data(), &QDtls::pskRequired, this, &DTLSClientListener::pskRequired);

                uc->setHostname(sslConfiguration->getSslPeerNameSNI());
                uc->setDtlscontext(crypto);

                if (sockets.size() > 100000)
                    emit log(tr("The number of DTLS client \"connections\" as reached 100000. For real ?"),ID, Pip3lineConst::LERROR);


                emit log(tr("Opening a new connection: %1").arg(sid),ID,Pip3lineConst::PLSTATUS);
                socket->connectToHost(hostAddress, hostPort);
                connect(socket, &QUdpSocket::readyRead, this, &DTLSClientListener::dataReceived);

                if (!crypto->doHandshake(socket)) {
                    emit log(tr("[%1] failed handshake: %2").arg(uc->getHostname(), crypto->dtlsErrorString()),ID, Pip3lineConst::LERROR);
                } else {
                    emit log(tr("[%1]: Handshake starting").arg(uc->getHostname()),ID, Pip3lineConst::PLSTATUS);
                    if (crypto->isConnectionEncrypted()) {
                        if (crypto->writeDatagramEncrypted(socket, data) != data.size()) {
                            emit log(tr("The DTLS packet was not send entirely '-_-: %1").arg(crypto->dtlsErrorString()),ID,Pip3lineConst::LWARNING);
                        }
                    } else {
                        qDebug() << "Handshake not completed yet";
                        QList<QByteArray> queue = waitingHandshakePackets.value(socket);
                        queue.append(data);
                        waitingHandshakePackets.insert(socket,queue);
                    }
                }
            } else {
                if (socket->writeDatagram(data,hostAddress,hostPort) != data.size()) {
                    emit log(tr("The UDP packet was not send entirely '-_-: %1").arg(socket->errorString()),ID,Pip3lineConst::LWARNING);
                }
            }

            updateConnectionsInfo();
        }
    } else {
        emit log(tr("The DTLS client is not started, cannot send the packet"),ID,Pip3lineConst::LERROR);
    }

    delete block;
}

bool DTLSClientListener::startListening()
{
    if (!running) {
        // really basic, a client listener never fails to "start"
        running = true;
        emit started();
        updateConnectionsInfo();
    }

    return true;
}

void DTLSClientListener::stopListening()
{
    if (running) {
        running = false;

        QHashIterator<QUdpSocket *, QSharedPointer<ConnectionDetails>> i(sockets);
        while (i.hasNext()) {
            i.next();
            QSharedPointer<ConnectionDetails> cd = i.value();
            BlocksSource::releaseID(cd->getSid());
            if (!cd->getDtlscontext().isNull() && cd->getDtlscontext()->isConnectionEncrypted()) {
                if (!cd->getDtlscontext()->shutdown(i.key())) {
                    emit log(tr("Error while shutting down DTLS socket: %2").arg(i.key()->errorString()), ID, Pip3lineConst::LERROR);
                }
            }
            delete i.key();
        }

        sockets.clear();
        mapExtSourcesToLocal.clear();
        triggerUpdate();
        emit stopped();
    }
}

void DTLSClientListener::onConnectionClosed(int cid)
{
    // checking mappings
    if (mapExtSourcesToLocal.contains(cid)) {
        cid = mapExtSourcesToLocal.take(cid);
    }

    QUdpSocket * socket = nullptr;
    QHashIterator<QUdpSocket *, QSharedPointer<ConnectionDetails>> i(sockets);
    while (i.hasNext()) {
        i.next();
        QSharedPointer<ConnectionDetails> cd = i.value();
        if (cd->getSid() == cid) {
            socket = i.key();
            socket->disconnectFromHost();
            QSharedPointer<QDtls> crypto = cd->getDtlscontext();
            if (!crypto.isNull() && crypto->isConnectionEncrypted()) {
                crypto->shutdown(socket);
            }
            break;
        }
    }

    if (socket != nullptr) {
        sockets.remove(socket);
        delete socket;

        BlocksSource::releaseID(cid);
        updateConnectionsInfo();
    } else {
        qDebug() << "[TLSClientListener::onConnectionClosed] Socket not found" << cid;
    }
}

void DTLSClientListener::createConnection()
{
    qDebug() << "create connection";
    sid = BlocksSource::newSourceID(this);

    QUdpSocket * socket = new(std::nothrow) QUdpSocket();
    if (socket == nullptr) {
        qFatal("Cannot allocate memory for QUdpSocket X{");
    }
    connect(socket, &QUdpSocket::readyRead, this, &DTLSClientListener::dataReceived);

    if (!socket->bind(hostAddress)) {
        emit log(tr("Binding socket error: %1 (packet dropped)").arg(socket->errorString()), ID, Pip3lineConst::LERROR);
        delete socket;
        return;
    }
    socket->connectToHost(hostAddress, hostPort);

    QSharedPointer<QDtls> crypto = QSharedPointer<QDtls>(new(std::nothrow) QDtls(QSslSocket::SslClientMode));
    crypto->setPeer(hostAddress, hostPort, sslConfiguration->getSslPeerNameSNI());
    crypto->setDtlsConfiguration(sslConfiguration->getSslConfiguration());
    crypto->setMtuHint(1200);

    connect(crypto.data(), &QDtls::handshakeTimeout, this, &DTLSClientListener::handshakeTimeout);
    connect(crypto.data(), &QDtls::pskRequired, this, &DTLSClientListener::pskRequired);

    sid = BlocksSource::newSourceID(this);
    QSharedPointer<ConnectionDetails> uc(new(std::nothrow) ConnectionDetails(hostAddress, hostPort));
    uc->setSid(sid);
    uc->setHostname(sslConfiguration->getSslPeerNameSNI());
    uc->setDtlscontext(crypto);
    sockets.insert(socket,uc);

    if (sockets.size() > 100000)
        emit log(tr("The number of DTLS client \"connections\" as reached 100000. For real ?"),ID, Pip3lineConst::LERROR);

    //mapExtSourcesToLocal.insert(bid, sid);
    emit log(tr("Opening a new connection: %1").arg(sid),ID,Pip3lineConst::PLSTATUS);
    connect(socket, &QUdpSocket::readyRead, this, &DTLSClientListener::dataReceived);

    if (!crypto->doHandshake(socket))
            emit log(tr("[%1:%2] failed handshake: %3").arg(hostAddress.toString()).arg(hostPort).arg(crypto->dtlsErrorString()),ID, Pip3lineConst::LERROR);
        else
            emit log(tr("[%1:%2]: Handshake starting").arg(hostAddress.toString()).arg(hostPort),ID, Pip3lineConst::PLSTATUS);

    updateConnectionsInfo();
}

void DTLSClientListener::dataReceived()
{
    //qDebug() << " DTLSClientListener::dataReceived";
    QUdpSocket * socket = dynamic_cast<QUdpSocket *>(sender());
    if (socket == nullptr) {
        qCritical() << "[DTLSClientListener::dataReceived] Cannot cast object T_T";
        return;
    }

    if (!sockets.contains(socket)) {
        emit log("[DTLSClientListener::dataReceived] Cannot find the connections details for the socket T_T", ID, Pip3lineConst::LERROR);
        return;
    }

    if (socket->pendingDatagramSize() <= 0) {
        return; // nothing to see
    }

    QHostAddress senderIP;
    quint16 senderPort = {};
    Pip3lineCommon::DataProtocol proto = Pip3lineCommon::unknown_protocol;
    QSharedPointer<ConnectionDetails> cd = sockets.value(socket);
    int sid = cd->getSid();

    // server sid if present, same sid if not
    QHashIterator<int, int> i(mapExtSourcesToLocal);
    while (i.hasNext()) {
        i.next();
        if (i.value() == sid) {
            sid = i.key();
            break;
        }
    }

    QByteArray data(socket->pendingDatagramSize(), Qt::Uninitialized);
    const qint64 bytesRead = socket->readDatagram(data.data(), data.size(), &senderIP, &senderPort);
    if (bytesRead <= 0) {
        return; // nothingness
    }
    data.resize(bytesRead);

    if (static_cast<quint64>(data.size()) >= sizeof(DtlsCommon::DTLSMessage)) {
        QByteArray input = data;
        DtlsCommon::DTLSMessage * dtlsHeader = reinterpret_cast<DtlsCommon::DTLSMessage *>(data.data());
        quint64 length = __bswap_64(dtlsHeader->sequence_number_length) & 0xFFFF;
        if (dtlsHeader->majorVersion == 0xFE && // DTLS Major 1
            dtlsHeader->minorVersion >= 0xFD && // DTLS Minor 0-2
            static_cast<quint64>(input.size()) >= sizeof(DtlsCommon::DTLSMessage) + length ) {
            // DTLS datagram
            QSharedPointer<QDtls> dtlscontext = cd->getDtlscontext();

            qDebug() << "DTLS client record type" << DtlsCommon::getDTLSRecordTypeStr(dtlsHeader->type);

            if (dtlsHeader->type == DtlsCommon::HANDSHAKE) { // handshake packet

                if (dtlscontext.isNull()) { // all DTLS contexts should have been created beforehand, this should never happen
                    emit log(tr("[DTLSClientListener::dataReceived] Cannot find the DTLS details for [%1:%2] T_T").arg(senderIP.toString()).arg(senderPort),ID ,Pip3lineConst::LERROR);
                    return;
                }
                switch (dtlscontext->handshakeState()) {
                    case QDtls::HandshakeNotStarted:
                        emit log(tr("[%1:%2] DTLS client error: Handshake not started T_T").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                        break;
                    case QDtls::HandshakeInProgress:
                        //qDebug() << QString::fromUtf8(data.toHex());
                        if (!dtlscontext->doHandshake(socket, data)) {
                            emit log(tr("[%1:%2] DTLS error: %3 (%4)")
                                     .arg(senderIP.toString())
                                     .arg(senderPort)
                                     .arg(dtlscontext->dtlsErrorString())
                                     .arg(static_cast<int>(dtlscontext->dtlsError())), ID, Pip3lineConst::LERROR);
                            DtlsCommon::printSslErrors(dtlscontext->peerVerificationErrors());
                        } else if (dtlscontext->handshakeState() == QDtls::HandshakeComplete ){
                            qDebug() << "DTLS client handshake completed";
                            QList<QByteArray> lb = waitingHandshakePackets.value(socket);
                            for (int k = 0; k < lb.size(); k++) {
                                QByteArray wdata = lb.at(k);
                                if (!dtlscontext->writeDatagramEncrypted(socket,wdata)) {
                                   emit log(tr("The DTLS packet was not send entirely '-_-: %1").arg(dtlscontext->dtlsErrorString()),ID,Pip3lineConst::LWARNING);
                                }
                            }
                            waitingHandshakePackets.remove(socket);
                        } else {
                            qDebug() << "DTLS client handshake in progress";
                        }
                        break;
                    case QDtls::PeerVerificationFailed:
                        emit log(tr("[%1:%2] DTLS: Peer Verification Error").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                        break;
                    case QDtls::HandshakeComplete:
                        emit log(tr("[%1:%2] DTLS error: Handshake already completed but handshake packet received ?_?").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                        break;
                }
                // we don't forward handshake messages, they are part of the protocol
                return;
            } else if (dtlsHeader->type == DtlsCommon::APPLICATION_DATA){
                if (dtlscontext.isNull()) {
                    emit log(tr("[%1:%2] DTLS error: Application data received while handshake not started (old connection?)").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                    return;
                } else if (dtlscontext->isConnectionEncrypted()) {
                    data = dtlscontext->decryptDatagram(socket, data);
                    proto = Pip3lineCommon::dtls_protocol;
                    //  need to send the data
                } else {
                    emit log(tr("[%1:%2] DTLS error: Application data received while handshake not completed (spurious)").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                    return;
                }

            } else if (dtlsHeader->type == DtlsCommon::ALERT && !dtlscontext.isNull()) {
                if (dtlscontext.isNull()) {
                    emit log(tr("[%1:%2] DTLS error: Alert received while handshake not started ?_?").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                } else {
                    QString alertLevel;
                    QString alertDescription;
                    if (static_cast<quint64>(data.size()) >= sizeof(DtlsCommon::DTLSMessage) + sizeof(DtlsCommon::DTLSAlert)) {
                        data = data.mid(sizeof(DtlsCommon::DTLSMessage));
                        DtlsCommon::DTLSAlert * dtlsAlert = reinterpret_cast<DtlsCommon::DTLSAlert *>(data.data());
                        alertLevel = DtlsCommon::getDTLSLevelStr(dtlsAlert->level);
                        alertDescription = DtlsCommon::getDTLSAlertStr(dtlsAlert->description);

                    } else {
                        alertLevel = "Unknown";
                        alertDescription = "Unknown";
                        emit log(tr("[%1:%2] Invalid DTLS alert packet received, not enough data 0_0").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                    }
                    emit log(tr("[%1:%2] DTLS Alert received: Level=(%3) Descr=(%4)")
                             .arg(senderIP.toString())
                             .arg(senderPort)
                             .arg(alertLevel)
                             .arg(alertDescription), ID, Pip3lineConst::LERROR);
                    sockets.remove(socket);
                    delete socket;
                    emit connectionClosed(sid);
                    updateConnectionsInfo();
                }
                // An alert closes the "connection", nothing to do after that
                return;
            } else if (dtlsHeader->type == DtlsCommon::CHANGE_CIPHER_SPEC) {
                emit log(tr("[%1:%2] DTLS Change Cipher Spec received outside handshake, ignoring as per RFC").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                // ignoring as per RFC
                return;
            }
        } else { // not a DTLS record
            qDebug() << "does not have the right version or length";
        }
    } else {
        qDebug() << "does not have enough data for DTLS";
    }

    // now checking if it could be a STUN packet
    if (proto == Pip3lineCommon::unknown_protocol && static_cast<quint64>(data.size()) >= sizeof(DtlsCommon::STUNMessage)) {
        DtlsCommon::STUNMessage * stunheader = reinterpret_cast<DtlsCommon::STUNMessage *>(data.data());
        if (stunheader->magic == DtlsCommon::STUN_MAGIC && (stunheader->type & 0xC000) == 0) {
            qDebug() << "STUN datagram";
            // STUN datagram
            proto = Pip3lineCommon::stun_protocol;
            // no changes we just forward the packet
        }
    }

    if (proto == Pip3lineCommon::unknown_protocol) {
        qDebug() << "DTLS client unknown protocol" << sid;
    }

    data = applyInboundTransform(data);
    if (!data.isEmpty()) {
        Block * datab = new(std::nothrow) Block(data,sid);
        if (datab == nullptr) qFatal("Cannot allocate Block for UdpServerListener X{");
        datab->setProtocol(proto);
        datab->setMultiplexed(true);
        // we don't care about the ports, they all come from the server side
//        datab->setSrcPort(senderPort);
//        datab->setDstPort(hostPort);
        emit blockReceived(datab);
    } else {
        qDebug() << tr("[%1:%2] DTLS client Processed Data packet is empty, ignoring").arg(senderIP.toString()).arg(senderPort);
    }
}

void DTLSClientListener::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "[DTLSClientListener::onError]" << error;
    if (error != QAbstractSocket::RemoteHostClosedError) {
        QSslSocket * sobject = dynamic_cast<QSslSocket *>(sender());
        if (sobject != nullptr) {
            emit log(sobject->errorString(), ID, Pip3lineConst::LERROR);
        } else {
            qDebug() << "[DTLSClientListener::onError] a connection was closed by the remote client";
        }
    }
}

void DTLSClientListener::handshakeTimeout()
{
    QDtls *crypto = dynamic_cast<QDtls *>(sender());
    if (crypto != nullptr) {
        emit log(tr("[%1:%2]: handshake timeout, re-transmitting").arg(crypto->peerAddress().toString()).arg(crypto->peerPort()), ID, Pip3lineConst::LWARNING);
        QHashIterator<QUdpSocket *, QSharedPointer<ConnectionDetails>> i(sockets);
        while (i.hasNext()) {
            i.next();
            QSharedPointer<ConnectionDetails>  cd = i.value();
            QUdpSocket *socket = i.key();
            if (cd->getTimeoutRetry() > 2) {
                int cid = cd->getSid();
                emit log(tr("[%1:%2]: handshake max timeout reached, removing connection").arg(crypto->peerAddress().toString()).arg(crypto->peerPort()), ID, Pip3lineConst::LERROR);
                QHashIterator<int,int> j(mapExtSourcesToLocal);
                while(j.hasNext()) {
                    j.next();
                    if (j.value() == cid) {
                        int rem = j.key();
                        mapExtSourcesToLocal.remove(rem);
                        break;
                    }
                }
                BlocksSource::releaseID(cid);
                sockets.remove(socket);
                delete socket;
                updateConnectionsInfo();
                return;
            }
            if (cd->getDtlscontext().data() == crypto ) {
                cd->incrTimeoutRetries();
                if (!crypto->handleTimeout(socket))
                    emit log(tr("[%1:%2]: re-transmition failed: %3").arg(crypto->peerAddress().toString()).arg(crypto->peerPort()).arg(crypto->dtlsErrorString()), ID, Pip3lineConst::LERROR);
                return;
            }
        }
    } else {
        qCritical() << "[DTLSClientListener::handshakeTimeout] Cannot cast QDtls object T_T";
    }
}

void DTLSClientListener::pskRequired(QSslPreSharedKeyAuthenticator *auth)
{
    Q_ASSERT(auth);
    QDtls *crypto = dynamic_cast<QDtls *>(sender());
    if (crypto != nullptr) {
        emit log(tr("%1: PSK has been requested").arg(crypto->peerVerificationName()), ID, Pip3lineConst::LWARNING);
        QByteArray psk = sslConfiguration->getPsk();
        if (psk.isEmpty()) {
            emit log(tr("%1: No PSK specified, aborting the connection").arg(crypto->peerVerificationName()), ID, Pip3lineConst::LERROR);
            // TODO cleanup connections
        } else {
            auth->setIdentity(crypto->peerVerificationName().toUtf8());
            auth->setPreSharedKey(psk);
        }
    } else {
        qCritical() << "[DTLSClientListener::handshakeTimeout] Cannot cast object T_T";
    }
}

void DTLSClientListener::internalUpdateConnectionsInfo()
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
                .arg(QString("DTLS"));
        tac.setDescription(desc);
        tac.setSource(this);
        connectionsInfo.append(tac);

        QHashIterator<QUdpSocket *, QSharedPointer<ConnectionDetails>> i(sockets);
        while (i.hasNext()) {
            i.next();
            QSharedPointer<ConnectionDetails> cd = i.value();
            QString desc;
            desc = desc.append(QString("[%1] %2:%3/%4")
                    .arg(cd->getSid())
                    .arg(hostAddress.toString())
                    .arg(hostPort))
                    .arg(tr("DTLS"));
            Target<BlocksSource *> tac;
            tac.setDescription(desc);
            tac.setConnectionID(cd->getSid());
            tac.setSource(this);
            connectionsInfo.append(tac);
        }
    }
}

bool DTLSClientListener::getUseSrcPorts() const
{
    return useSrcPorts;
}

QHash<QString, QString> DTLSClientListener::getConfiguration()
{
    QHash<QString, QString> ret = IPBlocksSources::getConfiguration();
    ret.insert(USE_SRC_PORT, QString::number(useSrcPorts ? 1 : 0));
    return ret;
}

void DTLSClientListener::setConfiguration(const QHash<QString, QString> &conf)
{
    IPBlocksSources::setConfiguration(conf);
    bool ok = false;
    if (conf.contains(USE_SRC_PORT)) {
        int val = conf.value(USE_SRC_PORT).toInt(&ok);
        if (!ok || (val != 0 && val != 1)) {
            emit log(tr("Invalid value for %1").arg(USE_SRC_PORT),ID, Pip3lineConst::LERROR);
        } else {
            setUseSrcPorts(val == 1);
        }
    }
}

void DTLSClientListener::setUseSrcPorts(bool value)
{
    useSrcPorts = value;
}

QWidget *DTLSClientListener::requestGui(QWidget *parent)
{
    BaseBlockSourceWidget * base = dynamic_cast<BaseBlockSourceWidget *>(IPBlocksSources::requestGui(parent));

    if (base != nullptr) {
        DtlsClientListenerWidget * localGui = new(std::nothrow) DtlsClientListenerWidget(this, parent);
        connect(this, &DTLSClientListener::destroyed, localGui , &DtlsClientListenerWidget::deleteLater);
        if (localGui == nullptr) {
            qFatal("Cannot allocate memory for DtlsClientListenerWidget X{");
        }
        base->insertWidgetInGeneric(0, localGui);
    }

    return base;
}
