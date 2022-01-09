#include <QtGlobal>
#if QT_FEATURE_dtls != -1

#include "dtlsserverlistener.h"
#include <QThread>
#include "shared/guiconst.h"
#include "connectionswidget.h"
#include "pipelinecommon.h"
#include "shared/sslconf.h"
#include "baseblocksourcewidget.h"
#include "dtlsserverlistenerwidget.h"
#include "dtlscommon.h"
#include <QDebug>
#ifdef _MSC_VER
#include <stdlib.h>
#define __bswap_64(x) _byteswap_uint64(x)
#elif defined(__APPLE__)
#include <libkern/OSByteOrder.h>
#define __bswap_64(x) OSSwapInt64(x)
#endif

const quint16 DtlsServerListener::DEFAULT_PORT = 3443;
const QHostAddress DtlsServerListener::DEFAULT_ADDRESS = QHostAddress::LocalHost;
const QString DtlsServerListener::ID = QString("DTLS server");
const int DtlsServerListener::MAX_DATAGRAM_SIZE = 3000;
const QString DtlsServerListener::DTLS_VERIFICATION_ENABLED = "dtls_verification_enabled";

DtlsServerListener::DtlsServerListener(QHostAddress hostAddress, quint16 hostPort, QObject *parent) :
    IPBlocksSources(hostAddress,hostPort, parent),
    udpSocket(nullptr),
    dtlsVerificationEnabled(false)
{
    flags = REFLEXION_OPTIONS | TLS_OPTIONS | TLS_ENABLED;
    type = SERVER;

    setTlsEnable(true);
    QSslConfiguration config = QSslConfiguration::defaultDtlsConfiguration();
    config.setDtlsCookieVerificationEnabled(dtlsVerificationEnabled);
    config.setProtocol(QSsl::DtlsV1_0OrLater);
    sslConfiguration->setSslConfiguration(config);

    connect(&connectionsTimer, &QTimer::timeout, this, &DtlsServerListener::checkTimeouts);
    connectionsTimer.setInterval(GuiConst::DEFAULT_UDP_TIMEOUT_MS);
    connectionsTimer.moveToThread(&serverThread);

    connectionsTimer.moveToThread(&serverThread);
    updateConnectionsTimer.moveToThread(&serverThread);
    moveToThread(&serverThread);
    serverThread.start();
}

DtlsServerListener::~DtlsServerListener()
{
    serverThread.quit();
    serverThread.wait();
    clients.clear();
}

QString DtlsServerListener::getName()
{
    return ID;
}

QString DtlsServerListener::getDescription()
{
    return ID;
}

bool DtlsServerListener::isStarted()
{
    return (udpSocket != nullptr && udpSocket->state() == QAbstractSocket::BoundState);
}

int DtlsServerListener::getTargetIdFor(int sourceId)
{
    int targetId = Block::INVALID_ID;
    if (sourceId != Block::INVALID_ID) {
        for (int i = 0; i < clients.size(); i++) {
            int suid = clients.at(i)->getSid();
            if (sourceId == suid) {
                targetId = suid;
                break;
            }
        }
    }
    return targetId;
}

void DtlsServerListener::sendBlock(Block *block)
{
    if (udpSocket != nullptr) {
        QByteArray data = applyOutboundTransform(block->getData());

        if (data.size() > MAX_DATAGRAM_SIZE ) {
            emit log(tr("The UDP packet is too large. It will be truncated to %1 bytes")
                     .arg(MAX_DATAGRAM_SIZE),
                     ID,
                     Pip3lineConst::LWARNING);
            data  = data.mid(0, MAX_DATAGRAM_SIZE);
        }

        bool foundSource = false;
        for (int i = 0; i < clients.size(); i++) {
            QSharedPointer<ConnectionDetails> client = clients.at(i);
            if (block->getSourceid() == client->getSid()) {
                qint64 bwritten = 0;
                if (!block->isMultiplexed() || block->getProtocol() == Pip3lineCommon::dtls_protocol) {
                    QSharedPointer<QDtls> context = client->getDtlscontext();
                    if (!context.isNull()) {
                        bwritten = context->writeDatagramEncrypted(udpSocket, data);
                        if (bwritten == -1) {
                            emit log(tr("[%1:%2] DTLS Sending error: %3")
                                     .arg(client->getAddress().toString())
                                     .arg(client->getPort()).arg(context->dtlsErrorString()),ID,Pip3lineConst::LERROR);
                        } else {
                            client->bumpLastTimestamp();
                        }
                    } else {
                        emit log(tr("[%1:%2] DTLS Sending error: DDTLS context is null")
                                 .arg(client->getAddress().toString())
                                 .arg(client->getPort())
                                 ,ID,Pip3lineConst::LERROR);
                    }
                } else {
                    bwritten = udpSocket->writeDatagram(data, client->getAddress(), client->getPort());
                    if (bwritten == -1) {
                        emit log(tr("[%1:%2] UDP Sending error: %3")
                                 .arg(client->getAddress().toString())
                                 .arg(client->getPort()).arg(udpSocket->errorString()),ID,Pip3lineConst::LERROR);
                    } else {
                        client->bumpLastTimestamp();
                    }
                }

                if (bwritten > -1 && bwritten != data.size()) {
                    emit log(tr("The UDP packet was not send entirely '-_-"),ID,Pip3lineConst::LWARNING);
                }

                foundSource = true;
                break;
            }
        }

        if (!foundSource) {
            emit log(tr("The DTLS client %1 cannot be found, cannot send the packet T_T").arg(block->getSourceid()),ID,Pip3lineConst::LERROR);
        }
    } else {
        qCritical() << tr("[DtlsServerListener::sendBlock]: socket is nullptr, cannot send the packet");
    }

    delete block;
}

bool DtlsServerListener::startListening()
{
    connectionsTimer.start();

    if (udpSocket != nullptr) { // already listening
        return true;
    }

    qWarning() << this->metaObject()->className() << "startListening";

    udpSocket = new(std::nothrow) QUdpSocket(this);
    if (udpSocket == nullptr) {
        qFatal("Cannot allocate memory for udpSocket X{");
    }

    udpSocket->moveToThread(&serverThread);

    connect(udpSocket, &QUdpSocket::readyRead, this, &DtlsServerListener::packetReceived);

    if (!udpSocket->bind(hostAddress, hostPort)) {
        emit log(tr("%1 error: %2").arg(getDescription()).arg(udpSocket->errorString()),ID,Pip3lineConst::LERROR);
        delete udpSocket;
        udpSocket = nullptr;
        return false;
    }

    emit log(tr("%1 started %2:%3").arg(getDescription()).arg(hostAddress.toString()).arg(hostPort), ID, Pip3lineConst::PLSTATUS);

    emit started();

    return true;
}

void DtlsServerListener::stopListening()
{
    if (udpSocket != nullptr) {

        for (int i = 0 ; i < clients.size(); i++) {
            QSharedPointer<ConnectionDetails> cd = clients.at(i);
            BlocksSource::releaseID(cd->getSid());
            if (!cd->getDtlscontext().isNull() && cd->getDtlscontext()->isConnectionEncrypted()) {
                cd->getDtlscontext()->shutdown(udpSocket);
            }
        }
        clients.clear();

        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;

        emit stopped();
        triggerUpdate();
    }
    connectionsTimer.stop();
}

void DtlsServerListener::packetReceived()
{
    QByteArray data;
    QHostAddress senderIP;
    quint16 senderPort = {};

    qint64 datagramSize = udpSocket->pendingDatagramSize();
    if (datagramSize < INT_MAX) {
        data.resize(static_cast<int>(datagramSize));
    } else {
        qCritical() << tr("[DtlsServerListener::dataReceived] datagramSize invalid T_T");
        return;
    }

    qint64 bread = udpSocket->readDatagram(data.data(), data.size(), &senderIP, &senderPort);

    if (bread == -1){
        emit log(tr("[%1:%2] UDP receiving error: %3")
                 .arg(senderIP.toString())
                 .arg(senderPort).arg(udpSocket->errorString()), ID, Pip3lineConst::LERROR);
        return;
    }

    if (bread == 0){
        emit log(tr("Received data block is empty, ignoring."), ID, Pip3lineConst::LERROR);
        return;
    }

    if (bread < datagramSize) {
        qCritical() << tr("[DtlsServerListener::dataReceived] not all the data was read T_T");
        data.resize(bread);
    }

    QSharedPointer<ConnectionDetails> client;

    int sid = Block::INVALID_ID;
    int index = clients.connectionIndex(senderIP,senderPort);

    if (index > Block::INVALID_ID) {
        client = clients.at(index);
        sid = client->getSid();
    } else {
        if (clients.size() > 1000000) {
            emit log(tr("The number of DTLS server \"connections\" as reached 1 Million. Dude for real ?"),ID, Pip3lineConst::LERROR);
        }
        sid = newSourceID(this);
        emit log(tr("New client: %1:%2/udp").arg(senderIP.toString()).arg(senderPort),ID, Pip3lineConst::LWARNING);
        client = QSharedPointer<ConnectionDetails>(new(std::nothrow) ConnectionDetails(senderIP,senderPort));
        client->setSid(sid);
        clients.append(client);
        updateConnectionsInfo();
    }
    client->bumpLastTimestamp();

    Pip3lineCommon::DataProtocol proto = Pip3lineCommon::unknown_protocol;

    // check if this is a potential DTLS packet
    if (static_cast<quint64>(data.size()) >= sizeof(DtlsCommon::DTLSMessage)) {
        QByteArray input = data;
        DtlsCommon::DTLSMessage * dtlsHeader = reinterpret_cast<DtlsCommon::DTLSMessage *>(data.data());
        quint64 length = __bswap_64(dtlsHeader->sequence_number_length) & 0xFFFF;
        if (dtlsHeader->majorVersion == 0xFE && // DTLS Major 1
            dtlsHeader->minorVersion >= 0xFD && // DTLS Minor 0-2
            static_cast<quint64>(input.size()) >= sizeof(DtlsCommon::DTLSMessage) + length ) {
            // DTLS datagram
            QSharedPointer<QDtls> dtlscontext = client->getDtlscontext();
            qDebug() << "DTLS record type" << DtlsCommon::getDTLSRecordTypeStr(dtlsHeader->type);

            if (dtlsHeader->type == DtlsCommon::HANDSHAKE || dtlsHeader->type == DtlsCommon::CHANGE_CIPHER_SPEC) { // handshake packet

                if (dtlscontext.isNull()) { // new dtls context
                    // qDebug() << tr("New DTLS client [%1:%2]").arg(senderIP.toString()).arg(senderPort);
                    dtlscontext = QSharedPointer<QDtls>(new(std::nothrow) QDtls(QSslSocket::SslServerMode));
                    dtlscontext->setPeer(senderIP, senderPort);
                    dtlscontext->setMtuHint(1200);
                    QSslConfiguration sslconf = sslConfiguration->getSslConfiguration();
                    sslconf.setDtlsCookieVerificationEnabled(dtlsVerificationEnabled);
                    dtlscontext->setDtlsConfiguration(sslconf);
                    dtlscontext->setCookieGeneratorParameters(verifier.cookieGeneratorParameters());
                    client->setDtlscontext(dtlscontext);
                }
                switch (dtlscontext->handshakeState()) {
                    case QDtls::HandshakeNotStarted:
                        if (dtlsVerificationEnabled) {
                            if (verifiedclient.contains(QPair<QHostAddress, quint16>(senderIP, senderPort))) {
                                qDebug() << tr("DTLS client verified [%1:%2]").arg(senderIP.toString()).arg(senderPort);
                            } else if (!verifier.verifyClient(udpSocket, data, senderIP, senderPort)) {
                                qCritical() << tr("DTLS Verfier error: ") + verifier.dtlsErrorString();
                                break;
                            } else {
                                verifiedclient.append(QPair<QHostAddress, quint16>(senderIP, senderPort));
                                qDebug() << tr("DTLS client verification started [%1:%2]").arg(senderIP.toString()).arg(senderPort);
                                break;
                            }
                        }
#if defined(__GNUC__) || defined(__GNUG__)
                        [[gnu::fallthrough]];
#elif defined(__clang__)
                        [[clang::fallthrough]];
#endif
                    case QDtls::HandshakeInProgress:
                        //6qDebug() << QString::fromUtf8(data.toHex());
                        if (!dtlscontext->doHandshake(udpSocket, data)) {
                            emit log(tr("[%1:%2] DTLS error: %3 (%4)")
                                     .arg(senderIP.toString())
                                     .arg(senderPort)
                                     .arg(dtlscontext->dtlsErrorString())
                                     .arg(static_cast<int>(dtlscontext->dtlsError())), ID, Pip3lineConst::LERROR);
                            DtlsCommon::printSslErrors(dtlscontext->peerVerificationErrors());
                        } else {
                            emit log(tr("DTLS session created for: %1:%2").arg(senderIP.toString()).arg(senderPort),ID, Pip3lineConst::LWARNING);
                           // qDebug() << "DTLS handshake approved";
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
                    emit log(tr("[%1:%2] DTLS error: Application data received while handshake not started (old client?)").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                    return;
                } else if (dtlscontext->isConnectionEncrypted()) {
                    data = dtlscontext->decryptDatagram(udpSocket, data);
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
                    emit log(tr("[%1:%2] DTLS Alert received")
                             .arg(senderIP.toString())
                             .arg(senderPort), ID, Pip3lineConst::LERROR);
                    if (index > Block::INVALID_ID) {
                        clients.removeAt(index); // removing connection data
                        emit connectionClosed(sid);
                    }
                }
                // An alert closes the "connection", nothing to do after that
                return;
            } else if (dtlsHeader->type == DtlsCommon::CHANGE_CIPHER_SPEC) {
                emit log(tr("[%1:%2] DTLS Change Cipher Spec received outside handshake, ignoring as per RFC").arg(senderIP.toString()).arg(senderPort), ID, Pip3lineConst::LERROR);
                // ignoring as per RFC
                return;
            }
        } else { // not a DTLS record
            //qDebug() << "does not have the right version or length";
        }
    } else {
        //qDebug() << "does not have enough data for DTLS";
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
        qDebug() << "DTLS server unknown protocol" << sid;
    }

    data = applyInboundTransform(data);
    if (!data.isEmpty()) {
        Block * datab = new(std::nothrow) Block(data,sid);
        if (datab == nullptr) qFatal("Cannot allocate Block for UdpServerListener X{");
        datab->setProtocol(proto);
        datab->setSrcPort(senderPort);
        datab->setDstPort(hostPort);
        datab->setMultiplexed(true);
        emit blockReceived(datab);
    } else {
        qDebug() << tr("[%1:%2] Processed Data packet is empty, ignoring").arg(senderIP.toString()).arg(senderPort);
    }
}

void DtlsServerListener::checkTimeouts()
{
    QList<QSharedPointer<ConnectionDetails> > list = clients;
    bool listUpdated = false;
    qint64 current = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0 ; i < list.size(); i++) {
        QSharedPointer<ConnectionDetails> uc = list.at(i);
        if ((current - uc->getLastPacketTimeStamp().toMSecsSinceEpoch()) > GuiConst::DEFAULT_UDP_TIMEOUT_MS) {
            qDebug() << tr("DTLS server: Client timeout [%1:%2]").arg(uc->getAddress().toString()).arg(uc->getPort());
            BlocksSource::releaseID(uc->getSid());
            clients.removeAll(uc);
            listUpdated = true;
            emit connectionClosed(uc->getSid());
        }
    }

    if (listUpdated) {
        updateConnectionsInfo();
    }
}

void DtlsServerListener::handleDtlsHandshakeTimeOut()
{
    QDtls *source = dynamic_cast<QDtls *>(sender());
    if (source != nullptr) {
        source->handleTimeout(udpSocket);
    } else {
        qCritical() << "[DtlsServerListener::handleDtlsHandshakeTimeOut] Source is nullptr T_T";
    }
}

QWidget *DtlsServerListener::requestGui(QWidget *parent)
{
    BaseBlockSourceWidget * base = dynamic_cast<BaseBlockSourceWidget *>(IPBlocksSources::requestGui(parent));

    if (base != nullptr) {
        DtlsServerListenerWidget * localGui = new(std::nothrow) DtlsServerListenerWidget(this, parent);
        connect(this, &DtlsServerListener::destroyed, localGui , &DtlsServerListenerWidget::deleteLater);
        if (localGui == nullptr) {
            qFatal("Cannot allocate memory for DtlsServerListenerWidget X{");
        }
        base->insertWidgetInGeneric(0, localGui);
    }

    return base;
}

void DtlsServerListener::internalUpdateConnectionsInfo()
{
    connectionsInfo.clear();
    for (int i = 0 ; i < clients.size(); i++) {
        QSharedPointer<ConnectionDetails> client = clients.at(i);
        QString desc = client->getAddress().toString();
        desc.append(QString(":%1/dtls").arg(client->getPort()));
        Target<BlocksSource *> tac;
        tac.setDescription(desc);
        tac.setConnectionID(client->getSid());
        tac.setSource(this);
        connectionsInfo.append(tac);
    }
}

int DtlsServerListener::getClientIndex(const QHostAddress &clientAddress, const quint16 &clientPort)
{
    int ret = -1;
    for (int i = 0; i < clients.size(); i++) {
        QSharedPointer<ConnectionDetails> c = clients.at(i);
        if (c->getAddress() == clientAddress && c->getPort() == clientPort) {
            ret = i;
            break;
        }
    }
    return ret;
}

bool DtlsServerListener::getDtlsVerificationEnabled() const
{
    return dtlsVerificationEnabled;
}

void DtlsServerListener::setDtlsVerificationEnabled(bool value)
{
    if (dtlsVerificationEnabled != value) {
        dtlsVerificationEnabled = value;
        QSslConfiguration config = sslConfiguration->getSslConfiguration();
        config.setDtlsCookieVerificationEnabled(dtlsVerificationEnabled);
        config.setProtocol(QSsl::DtlsV1_0OrLater);
        sslConfiguration->setSslConfiguration(config);
    }
}

QHash<QString, QString> DtlsServerListener::getConfiguration()
{
    QHash<QString, QString> ret = IPBlocksSources::getConfiguration();
    ret.insert(DTLS_VERIFICATION_ENABLED, QString::number(dtlsVerificationEnabled ? 1 : 0));
    return ret;
}

void DtlsServerListener::setConfiguration(const QHash<QString, QString> &conf)
{
    IPBlocksSources::setConfiguration(conf);
    bool ok = false;
    if (conf.contains(DTLS_VERIFICATION_ENABLED)) {
        int val = conf.value(DTLS_VERIFICATION_ENABLED).toInt(&ok);
        if (!ok || (val != 0 && val != 1)) {
            emit log(tr("Invalid value for %1").arg(DTLS_VERIFICATION_ENABLED),ID, Pip3lineConst::LERROR);
        } else {
            setDtlsVerificationEnabled(val == 1);
        }
    }
}
#endif
