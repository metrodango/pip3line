#include "udpserverlistener.h"
#include <QThread>
#include "shared/guiconst.h"
#include "connectionswidget.h"
#include <QDebug>

const quint16 UdpServerListener::DEFAULT_PORT = 3000;
const QHostAddress UdpServerListener::DEFAULT_ADDRESS = QHostAddress::LocalHost;
const QString UdpServerListener::ID = QString("UDP server");
const int UdpServerListener::MAX_DATAGRAM_SIZE = 3000;

UdpServerListener::UdpServerListener(QHostAddress hostAddress, quint16 hostPort, QObject *parent) :
    IPBlocksSources(hostAddress,hostPort, parent)
{
    flags = REFLEXION_OPTIONS ;
    type = SERVER;
    udpSocket = nullptr;
    connect(&connectionsTimer, &QTimer::timeout, this, &UdpServerListener::checkTimeouts);
    connectionsTimer.setInterval(GuiConst::DEFAULT_UDP_TIMEOUT_MS);
    connectionsTimer.moveToThread(&serverThread);
    updateConnectionsTimer.moveToThread(&serverThread);
    moveToThread(&serverThread);
    serverThread.start();
}

UdpServerListener::~UdpServerListener()
{
    serverThread.quit();
    serverThread.wait();
}

QString UdpServerListener::getName()
{
    return ID;
}

QString UdpServerListener::getDescription( )
{
    return tr("UDP server");
}

bool UdpServerListener::isStarted()
{
    return (udpSocket != nullptr && udpSocket->state() == QAbstractSocket::BoundState);
}

int UdpServerListener::getTargetIdFor(int sourceId)
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

void UdpServerListener::sendBlock(Block *block)
{
    if (udpSocket != nullptr) {
        QByteArray data = applyOutboundTransform(block->getData());

        if (data.size() > MAX_DATAGRAM_SIZE ) {
            emit log(tr("The UDP packet is too large. It will be truncated to %1 bytes").arg(MAX_DATAGRAM_SIZE),
                     ID,
                     Pip3lineConst::LWARNING);
            data  = data.mid(0,MAX_DATAGRAM_SIZE);
        }

        bool foundSource = false;
        for (int i = 0; i < clients.size(); i++) {
            QSharedPointer<ConnectionDetails> client = clients.at(i);
            if (block->getSourceid() ==  client->getSid()) {
                qint64 bwritten = udpSocket->writeDatagram(data, client->getAddress(), client->getPort());
                if (bwritten == -1) {
                    emit log(tr("[%1:%2] UDP Sending error: %3")
                             .arg(client->getAddress().toString())
                             .arg(client->getPort()).arg(udpSocket->errorString()),ID,Pip3lineConst::LERROR);
                } else {
                    client->bumpLastTimestamp();
                }
                if (bwritten != data.size()) {
                    emit log(tr("The UDP packet was not send entirely '-_-"),ID,Pip3lineConst::LWARNING);
                }
                foundSource = true;

                break;
            }
        }

        if (!foundSource) {
            emit log(tr("The UDP client cannot be found, cannot send the packet T_T"),ID,Pip3lineConst::LERROR);
        }
    } else {
        qCritical() << tr("[UdpServerListener::sendBlock]: socket is nullptr, cannot send the packet");
    }

    delete block;
}

bool UdpServerListener::startListening()
{

    connectionsTimer.start();
    qWarning() << this->metaObject()->className() << "startListening";
    if (udpSocket != nullptr) { // already listening
        return true;
    }

    udpSocket = new(std::nothrow) QUdpSocket(this);
    if (udpSocket == nullptr) {
        qFatal("Cannot allocate memory for udpSocket X{");
    }

    udpSocket->moveToThread(&serverThread);

    connect(udpSocket, &QUdpSocket::readyRead, this, &UdpServerListener::packetReceived);

    if (!udpSocket->bind(hostAddress, hostPort)) {
        emit log(tr("UDP server error: %1").arg(udpSocket->errorString()), ID, Pip3lineConst::LERROR);
        delete udpSocket;
        udpSocket = nullptr;
        return false;
    }

    emit log(tr("UDP server started %1:%2").arg(hostAddress.toString()).arg(hostPort), ID, Pip3lineConst::PLSTATUS);

    emit started();

    return true;
}

void UdpServerListener::stopListening()
{
    if (udpSocket != nullptr) {
        udpSocket->close();
        delete udpSocket;
        udpSocket = nullptr;
        for (int i = 0 ; i < clients.size(); i++) {
            BlocksSource::releaseID(clients.at(i)->getSid());
        }
        clients.clear();
        emit stopped();
        triggerUpdate();
    }
    connectionsTimer.stop();
}

void UdpServerListener::packetReceived()
{
    QByteArray data;
    QHostAddress senderIP;
    quint16 senderPort;

    qint64 datagramSize = udpSocket->pendingDatagramSize();
    if (datagramSize < INT_MAX) {
        data.resize(static_cast<int>(datagramSize));
    } else {
        qCritical() << tr("[UdpServerListener::dataReceived] datagramSize invalid T_T");
        return;
    }

    qint64 bread = 0;
    bread += udpSocket->readDatagram(data.data(), data.size(), &senderIP, &senderPort);

    if (bread != datagramSize) {
        qCritical() << tr("[UdpClientListener::dataReceived] not all the data was read T_T");
    }

    if (data.isEmpty()){
        emit log(tr("Received data block is empty, ignoring."),ID, Pip3lineConst::LERROR);
        return;
    }

    QSharedPointer<ConnectionDetails> client;

    int sid = Block::INVALID_ID;
    int index = clients.connectionIndex(senderIP,senderPort);
    if (index > Block::INVALID_ID) {
        client = clients.at(index);
        sid = client->getSid();
    } else {
        if (clients.size() > 1000000)
            emit log(tr("The number of UDP server \"connections\" as reached 1 Millions. For real ?"),ID, Pip3lineConst::LERROR);
        client = QSharedPointer<ConnectionDetails>(new(std::nothrow) ConnectionDetails(senderIP,senderPort));
        sid = newSourceID(this);
        client->setSid(sid);
        clients.append(client);
        updateConnectionsInfo();
    }
    client->bumpLastTimestamp();

    data = applyInboundTransform(data);
    if (!data.isEmpty()) {
        Block * datab = new(std::nothrow) Block(data,sid);
        if (datab == nullptr) qFatal("Cannot allocate Block for UdpServerListener X{");

        emit blockReceived(datab);
    } else {
        qDebug() << tr("[%1:%2] Processed Data packet is empty, ignoring").arg(senderIP.toString()).arg(senderPort);
    }
}

void UdpServerListener::checkTimeouts()
{
    QList<QSharedPointer<ConnectionDetails> > list = clients; // copying we don't want to interfer with the original yet
    bool listUpdated = false;
    qint64 current = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0 ; i < list.size(); i++) {
        QSharedPointer<ConnectionDetails> uc = list.at(i);
        if ((current - uc->getLastPacketTimeStamp().toMSecsSinceEpoch()) > GuiConst::DEFAULT_UDP_TIMEOUT_MS) {
            qDebug() << tr("UDP Client timeout [%2:%1]").arg(uc->getAddress().toString()).arg(uc->getPort());
            BlocksSource::releaseID(uc->getSid());
            clients.removeAll(uc);
            listUpdated = true;
        }
    }

    if (listUpdated) {
        updateConnectionsInfo();
    }
}

void UdpServerListener::internalUpdateConnectionsInfo()
{
    connectionsInfo.clear();
    for (int i = 0 ; i < clients.size(); i++) {
        QSharedPointer<ConnectionDetails> client = clients.at(i);
        QString desc = client->getAddress().toString();
        desc.append(QString(":%1/udp").arg(client->getPort()));
        Target<BlocksSource *> tac;
        tac.setDescription(desc);
        tac.setConnectionID(client->getSid());
        tac.setSource(this);
        connectionsInfo.append(tac);
    }
}

