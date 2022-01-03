#include "connectionswidget.h"
#include "udpclientlistener.h"
#include "shared/guiconst.h"
#include <QTabWidget>
#include <QDebug>

const int UdpClientListener::MAX_UDP_DATAGRAM_SIZE_HARD = 65507; // practical limit of a UDP packet size;
const quint16 UdpClientListener::DEFAULT_PORT = 53;
const QHostAddress UdpClientListener::DEFAULT_ADDRESS = QHostAddress("192.168.23.1");
const QString UdpClientListener::ID = QString("UDP client");

UdpClientListener::UdpClientListener(QHostAddress hostAddress, quint16 hostPort, QObject *parent) :
    IPBlocksSources(hostAddress,hostPort, parent),
    running(false)
{
    flags = REFLEXION_OPTIONS | GEN_IP_OPTIONS;
    type = CLIENT;
    connect(&connectionsTimer, &QTimer::timeout, this, &UdpClientListener::checkTimeouts);
    connectionsTimer.setInterval(GuiConst::DEFAULT_UDP_TIMEOUT_MS);
    connectionsTimer.moveToThread(&workerThread);

    updateConnectionsTimer.moveToThread(&workerThread);
    moveToThread(&workerThread);
    workerThread.start();
}

UdpClientListener::~UdpClientListener()
{

    // at this point all the sockets should have been cleared already !!!
    workerThread.quit();
    workerThread.wait();

    mapExtSourcesToLocal.clear();
}

QString UdpClientListener::getName()
{
    return ID;
}

QString UdpClientListener::getDescription()
{
    return tr("Basic UDP client");
}

bool UdpClientListener::isStarted()
{
    return running;
}

int UdpClientListener::getTargetIdFor(int sourceId)
{
    int targetId = Block::INVALID_ID;
    if (sourceId != Block::INVALID_ID) {
        int mappedSid = Block::INVALID_ID;
        if (mapExtSourcesToLocal.contains(sourceId)) {
            mappedSid = mapExtSourcesToLocal.value(sourceId);
        }

        foreach (ConnectionDetails cd , udpSockets) {
            int suid = cd.getSid();
            // either we are sending directly to the blocksource or getting the block from another one
            if (sourceId == suid || mappedSid == suid) {
                targetId = suid;
                break;
            }
        }
    }
    return targetId;
}

void UdpClientListener::sendBlock(Block *block)
{
    if (running) {
        QByteArray data = applyOutboundTransform(block->getData());
        if (data.size() > MAX_UDP_DATAGRAM_SIZE_HARD ) {
            emit log(tr("The UDP packet is too large. It will be truncated to %1 bytes").arg(MAX_UDP_DATAGRAM_SIZE_HARD), ID, Pip3lineConst::LWARNING);
            data  = data.mid(0,MAX_UDP_DATAGRAM_SIZE_HARD);
        }

        int sid = -1;
        int bid = block->getSourceid();
        if (mapExtSourcesToLocal.contains(bid)) {
            sid = mapExtSourcesToLocal.value(bid);
        }

        bool foundSource = false;

        QHashIterator<QUdpSocket *, ConnectionDetails> i(udpSockets);
        while (i.hasNext()) {
            i.next();
            int suid = i.value().getSid();
            if (bid == suid || sid == suid) { // either we are sending directly to the blocksource or getting the block from another one
                qint64 bwritten = i.key()->writeDatagram(data,hostAddress,hostPort);
                if (bwritten != data.size()) {
                    emit log(tr("The UDP packet was not send entirely '-_-"), ID, Pip3lineConst::LWARNING);
                }
                foundSource = true;
            }
        }

        if (!foundSource) { // or we open a new connection

            QUdpSocket * socket = new(std::nothrow) QUdpSocket();
            if (socket == nullptr) {
                qFatal("Cannot allocate memory for QUdpSocket X{");
            }
            connect(socket, &QUdpSocket::readyRead, this, &UdpClientListener::dataReceived);
            sid = BlocksSource::newSourceID(this);
            ConnectionDetails uc(hostAddress, hostPort);
            uc.setSid(sid);
            udpSockets.insert(socket,uc);

            if (udpSockets.size() > 1000000)
                emit log(tr("The number of UDP client \"connections\" as reached 1 Millions. Dude for real ?"),ID, Pip3lineConst::LERROR);
            mapExtSourcesToLocal.insert(bid, sid);
            emit log(tr("Opening a new connection: %1").arg(sid),ID,Pip3lineConst::PLSTATUS);

            if (socket->writeDatagram(data,hostAddress,hostPort) != data.size()) {
                emit log(tr("The UDP packet was not send entirely '-_-"),ID,Pip3lineConst::LWARNING);
            }

            updateConnectionsInfo();
        }
    } else {
        emit log(tr("The UDP client is not started, cannot send the packet"),ID,Pip3lineConst::LERROR);
    }

    delete block;
}

bool UdpClientListener::startListening()
{
    qDebug() << tr("[UdpClientListener::startListening]");
    if (!running) {

        connectionsTimer.start();
        // really basic
        running = true;
        emit started();
        updateConnectionsInfo();
    }

    return true;
}

void UdpClientListener::stopListening()
{
    connectionsTimer.stop();
    if (running) {
        running = false;
        qDebug() << tr("[UdpClientListener::stopListening]");
        QHashIterator<QUdpSocket *, ConnectionDetails> i(udpSockets);
        while (i.hasNext()) {
            i.next();
            BlocksSource::releaseID(i.value().getSid());
            delete i.key();
        }

        udpSockets.clear();
        mapExtSourcesToLocal.clear();

        emit stopped();
        triggerUpdate();
    }

}

void UdpClientListener::dataReceived()
{
    QObject *s = sender();

    if (s != nullptr) {
        QUdpSocket * socket = static_cast<QUdpSocket *>(s);
        if (!udpSockets.contains(socket)) {
            qCritical() << tr("[UdpClientListener::dataReceived] Unknown client T_T");
            return;
        }

        QByteArray data;
        QHostAddress sender;
        quint16 senderPort;

        qint64 datagramSize = socket->pendingDatagramSize();
        if (datagramSize < INT_MAX) {
            data.resize(static_cast<int>(datagramSize));
        } else {
            qCritical() << tr("[UdpClientListener::dataReceived] datagramSize invalid T_T");
            return;
        }

        qint64 bread = 0;

        bread = socket->readDatagram(data.data(), data.size(), &sender, &senderPort);
        if (bread != datagramSize) {
            qCritical() << tr("[UdpClientListener::dataReceived] not all the data was read ....");
        }

        data = applyInboundTransform(data);

        if (data.isEmpty()){
            emit log(tr("Received data block is empty, ignoring."), ID, Pip3lineConst::LERROR);
            return;
        }

        int sid = udpSockets.value(socket).getSid();
        Block * datab = new(std::nothrow) Block(data,sid);
        if (datab == nullptr) qFatal("Cannot allocate Block for UdpServerListener X{");

        QHashIterator<int, int> i(mapExtSourcesToLocal);
        while (i.hasNext()) {
            i.next();
            if (i.value() == sid)
                datab->setSourceid(i.key());
        }

        emit blockReceived(datab);
    } else {
        qCritical() << tr("[UdpClientListener::dataReceived] sender is nullptr");
    }
}

void UdpClientListener::checkTimeouts()
{
    bool listUpdated = false;
    qint64 current = QDateTime::currentMSecsSinceEpoch();

    QList <QUdpSocket *> list = udpSockets.keys();
    for (int i = 0; i < list.size(); i++) {
        QUdpSocket * socket = list.at(i);
        ConnectionDetails uc = udpSockets.value(list.at(i));
        if (uc.getPort() == 0) {
            qCritical() << tr("[UdpClientListener::checkTimeouts] null UDPClient returned T_T");
        }
        if ((current - uc.getCreationTimeStamp().toMSecsSinceEpoch()) > GuiConst::DEFAULT_UDP_TIMEOUT_MS) {
            qDebug() << tr("UDP Client timeout [%2:%1]").arg(uc.getAddress().toString()).arg(uc.getPort());
            BlocksSource::releaseID(uc.getSid());
            delete socket;
            udpSockets.remove(socket);
            listUpdated = true;
        }
    }

    if (listUpdated)
        updateConnectionsInfo();
}

void UdpClientListener::internalUpdateConnectionsInfo()
{
    connectionsInfo.clear();
    if (running) { // accepting new connections
        Target<BlocksSource *> tac;
        tac.setConnectionID(Block::INVALID_ID);
        QString desc;
        desc.append(QString("[%1]:%2:%3/udp")
                    .arg(BlocksSource::NEW_CONNECTION_STRING)
                    .arg(hostAddress.toString())
                    .arg(hostPort));
        tac.setDescription(desc);
        tac.setSource(this);
        connectionsInfo.append(tac);

        QHashIterator<QUdpSocket *, ConnectionDetails> i(udpSockets);
        while (i.hasNext()) {
            i.next();
            QString desc;
            desc.append(QString("[%1]:%2:%3/udp")
                     .arg(i.value().getSid())
                     .arg(hostAddress.toString())
                     .arg(hostPort));
            Target<BlocksSource *> tac;
            tac.setDescription(desc);
            tac.setConnectionID(i.value().getSid());
            tac.setSource(this);
            connectionsInfo.append(tac);
        }
    }
}

