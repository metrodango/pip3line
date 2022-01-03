#include "myoproxy.h"
#include "guihelper.h"
#include "myoproxywidget.h"
#include "sources/blocksources/tlsserverlistener.h"
#include "sources/blocksources/tlsclientlistener.h"
#include "sources/blocksources/udpclientlistener.h"
#include "sources/blocksources/udpserverlistener.h"
#include "sources/blocksources/pipeclientlistener.h"
#include "sources/blocksources/sharedmemorysource.h"
#include "sources/blocksources/fileblocksource.h"
#include "shared/guiconst.h"
#include <QDebug>

const QStringList MYOProxy::SERVERS_LIST = MYOProxy::initServerList();
const QStringList MYOProxy::CLIENTS_LIST = MYOProxy::initClientList();

MYOProxy::MYOProxy(GuiHelper *guiHelper, QObject *parent) :
    SourcesOrchestatorAbstract(parent),
    guiHelper(guiHelper)
{
    serverType = SERVER_INVALID;
    clientType = CLIENTS_INVALID;
    hasDirection = true;
    forwarder = true;
}

MYOProxy::~MYOProxy()
{
    delete serverSource;
    delete clientSource;
}

QList<Target<SourcesOrchestatorAbstract *> > MYOProxy::getAvailableConnections()
{
    QList<Target<SourcesOrchestatorAbstract *> > ret;
    if (serverSource != nullptr) {
        ret.append(toOrchestratorTargetList(serverSource->getAvailableConnections()));
    }

    if (clientSource != nullptr) {
        ret.append(toOrchestratorTargetList(clientSource->getAvailableConnections()));
    }
    return ret;
}

BlocksSource *MYOProxy::getBlockSource(int index)
{
    if (index == 0) {
        return serverSource;
    } else if (index == 1) {
        return clientSource;
    }

    return nullptr;
}

int MYOProxy::blockSourceCount() const
{
    return 2; // it's a proxy ...
}

void MYOProxy::setServer(MYOProxy::SERVERS serverVal)
{
    if (serverType == serverVal) return; // nothing to see here

    delete serverSource;

    serverType = serverVal;

    switch (serverVal) {
        case SERVER_TCP:
            serverSource = new(std::nothrow) TLSServerListener();
            if (serverSource == nullptr) qFatal("Cannot allocate memory for TLSServerListener X{");
            break;
        case SERVER_UDP:
            serverSource = new(std::nothrow) UdpServerListener();
            if (serverSource == nullptr) qFatal("Cannot allocate memory for UdpServerListener X{");
            break;
        case FILE_SOURCE:
            serverSource = new (std::nothrow) FileBlockSource(FileBlockSource::Reader);
            if (serverSource == nullptr) qFatal("Cannot allocate memory for FileBlockSource X{");
            break;
        default:
            qCritical() << tr("[MYOProxy::setServer] Invalid server type");
            return;
    }

    connect(this, &MYOProxy::startChildren, serverSource, &BlocksSource::startListening, Qt::QueuedConnection);
    connect(this, &MYOProxy::stopChildren, serverSource, &BlocksSource::stopListening, Qt::QueuedConnection);
    connect(this, &MYOProxy::resetChildren, serverSource, &BlocksSource::restart, Qt::QueuedConnection);
    connect(serverSource, &BlocksSource::started, this, &MYOProxy::started);
    connect(serverSource, &BlocksSource::stopped, this, &MYOProxy::stopped);
    connect(serverSource, &BlocksSource::blockReceived, this, &MYOProxy::onBlockReceived);
    connect(serverSource, &BlocksSource::log, this, &MYOProxy::log, Qt::QueuedConnection);
    connect(serverSource, &BlocksSource::updated, this, &MYOProxy::connectionsChanged, Qt::QueuedConnection);
    connect(serverSource, &BlocksSource::newConnection, this, &MYOProxy::connectionsChanged, Qt::QueuedConnection);
    connect(serverSource, &BlocksSource::inboundTranformSelectionRequested, guiHelper, &GuiHelper::onInboundTransformRequested);
    connect(serverSource, &BlocksSource::outboundTranformSelectionRequested, guiHelper, &GuiHelper::onOutboundTransformRequested);

    if (clientSource != nullptr) {
        connect(serverSource, &BlocksSource::connectionClosed, clientSource, &BlocksSource::onConnectionClosed, Qt::QueuedConnection);
        connect(clientSource, &BlocksSource::connectionClosed, serverSource, &BlocksSource::onConnectionClosed, Qt::QueuedConnection);
    }

    MYOProxyWidget *myop = dynamic_cast<MYOProxyWidget *>(confgui);
    if (myop != nullptr) {
        myop->setServerWidget(serverSource->getGui());
    }
}

void MYOProxy::setClient(MYOProxy::CLIENTS clientVal)
{
    if (clientType == clientVal) return; // nothing to see here

    delete clientSource;
    clientType = clientVal;

    switch (clientType) {
        case CLIENT_TCP:
            clientSource = new(std::nothrow) TLSClientListener();
            if (clientSource == nullptr) qFatal("Cannot allocate memory for TLSClientListener X{");
            break;
        case CLIENT_UDP:
            clientSource = new(std::nothrow) UdpClientListener();
            if (clientSource == nullptr) qFatal("Cannot allocate memory for UdpClientListener X{");
            break;
        case CLIENT_PIPE:
            clientSource = new(std::nothrow) PipeClientListener();
            if (clientSource == nullptr) qFatal("Cannot allocate memory for PipeClientListener X{");
            break;
        case SHARED_MEM:
            clientSource = new(std::nothrow) SharedMemorySource();
            if (clientSource == nullptr) qFatal("Cannot allocate memory for SharedMemorySource X{");
            break;
        case FILE_SINK:
            clientSource = new(std::nothrow) FileBlockSource(FileBlockSource::Writer);
            if (clientSource == nullptr) qFatal("Cannot allocate memory for FileSourceSink X{");
            break;
        default:
            qCritical() << tr("[MYOProxy::setClient] Invalid client type");
            return;
    }

    connect(this, &MYOProxy::startChildren, clientSource, &BlocksSource::startListening, Qt::QueuedConnection);
    connect(this, &MYOProxy::stopChildren, clientSource, &BlocksSource::stopListening, Qt::QueuedConnection);
    connect(this, &MYOProxy::resetChildren, clientSource, &BlocksSource::restart, Qt::QueuedConnection);
    connect(clientSource, &BlocksSource::started, this, &MYOProxy::started);
    connect(clientSource, &BlocksSource::stopped, this, &MYOProxy::stopped);
    connect(clientSource, &BlocksSource::blockReceived, this, &MYOProxy::onBlockReceived);
    connect(clientSource, &BlocksSource::log, this, &MYOProxy::log, Qt::QueuedConnection);
    connect(clientSource, &BlocksSource::updated, this, &MYOProxy::connectionsChanged, Qt::QueuedConnection);
    connect(clientSource, &BlocksSource::newConnection, this, &MYOProxy::connectionsChanged, Qt::QueuedConnection);

    connect(clientSource, &BlocksSource::inboundTranformSelectionRequested, guiHelper, &GuiHelper::onInboundTransformRequested);
    connect(clientSource, &BlocksSource::outboundTranformSelectionRequested, guiHelper, &GuiHelper::onOutboundTransformRequested);

    if (serverSource != nullptr) {
        connect(serverSource, &BlocksSource::connectionClosed, clientSource, &BlocksSource::onConnectionClosed, Qt::QueuedConnection);
        connect(clientSource, &BlocksSource::connectionClosed, serverSource, &BlocksSource::onConnectionClosed, Qt::QueuedConnection);
    }

    MYOProxyWidget *myop = dynamic_cast<MYOProxyWidget *>(confgui);
    if (myop != nullptr) {
        myop->setClientWidget(clientSource->getGui());
    }
}

QHash<QString, QString> MYOProxy::getConfiguration()
{
    QHash<QString, QString> ret = SourcesOrchestatorAbstract::getConfiguration();

    ret.insert(GuiConst::STATE_SERVER_TYPE, QString::number(serverType));
    ret.insert(GuiConst::STATE_CLIENT_TYPE, QString::number(clientType));

    return ret;
}

void MYOProxy::setConfiguration(QHash<QString, QString> conf)
{
    SourcesOrchestatorAbstract::setConfiguration(conf);
    bool ok = false;

    if (conf.contains(GuiConst::STATE_SERVER_TYPE)) {
        int num = conf.value(GuiConst::STATE_SERVER_TYPE).toInt(&ok);
        if (ok && (num == SERVER_TCP ||
                   num == SERVER_UDP ||
                   num == FILE_SOURCE)) {
            setServer(static_cast<MYOProxy::SERVERS>(num));
        }
    }

    if (conf.contains(GuiConst::STATE_CLIENT_TYPE)) {
        int num = conf.value(GuiConst::STATE_CLIENT_TYPE).toInt(&ok);
        if (ok && (num == CLIENT_TCP ||
                   num == CLIENT_UDP ||
                   num == CLIENT_PIPE ||
                   num == SHARED_MEM ||
                   num == FILE_SINK)) {
            setClient(static_cast<MYOProxy::CLIENTS>(num));
        }
    }
}

void MYOProxy::postPacket(QSharedPointer<Packet> packet)
{
    if (serverSource == nullptr){
        qCritical() << tr("[MYOProxy::postPacket] Server source is null T_T");
        return;
    }

    if (clientSource == nullptr ) {
        qCritical() << tr("[MYOProxy::postPacket] Client source is null T_T");
        return;
    }

    SourcesOrchestatorAbstract::postPacket(packet);
    BlocksSource * source = BlocksSource::getSourceObject(packet->getSourceid());

    if (packet->getDirection() == Packet::LEFTRIGHT) {
        clientSource->postBlockForSending(packet->toBlock());
    } else if (packet->getDirection() == Packet::RIGHTLEFT) {
        serverSource->postBlockForSending(packet->toBlock());
    } else { // NODIRECTION GIVEN
        if (source == serverSource) {
            packet->setDirection(Packet::RIGHTLEFT);
            serverSource->postBlockForSending(packet->toBlock());
        } else if (source == clientSource || packet->getSourceid() == Block::INVALID_ID) { // if invalid id just send it to the client as new connection
            packet->setDirection(Packet::LEFTRIGHT);
            clientSource->postBlockForSending(packet->toBlock());
        } else {
            emit log(tr("Connection [%1] cannot be identified, the packet will not be send").arg(packet->getSourceid()),
                     metaObject()->className(),
                     Pip3lineConst::LERROR);
        }
    }
}

bool MYOProxy::start()
{
    if (serverSource == nullptr){
        qCritical() << tr("[MYOProxy::start] Server source is null T_T");
        return false;
    }

    if (clientSource == nullptr ) {
        qCritical() << tr("[MYOProxy::start] Client source is null T_T");
        return false;
    }

    emit startChildren();

    return true;
}

void MYOProxy::onBlockReceived(Block *block)
{
    BlocksSource * senderObj = dynamic_cast<BlocksSource *>(sender());
    if (senderObj != nullptr) {
        if (senderObj != serverSource && senderObj != clientSource) {
            qCritical() << tr("[MYOProxy::onBlockReceived] Cannot recognized the sender, dropping the block T_T");
            delete block;
            return;
        }

        QSharedPointer<Packet> pac = QSharedPointer<Packet> (new(std::nothrow) Packet(block));
        if (pac.isNull()) {
            qFatal("[MYOProxy::onBlockReceived] Cannot allocate Packet X{");
        }

        if (senderObj == serverSource) {
            pac->setDirection(Packet::LEFTRIGHT);
        } else {
            pac->setDirection(Packet::RIGHTLEFT);
        }

        emit newPacket(pac);
        delete block;
    } else {
        qCritical() << tr("[MYOProxy::onBlockReceived] Cannot cast the sender T_T");
    }
}

QWidget *MYOProxy::requestConfGui(QWidget *parent)
{
    MYOProxyWidget *myop = new (std::nothrow) MYOProxyWidget(this, parent);
    if (myop == nullptr) {
        qFatal("[MYOProxy::requestConfGui] Cannot allocate MYOProxyWidget X{");
    }

    if (serverSource != nullptr) {
        myop->setServerWidget(serverSource->getGui());
    }

    if (clientSource != nullptr) {
        myop->setClientWidget(clientSource->getGui());
    }

    return myop;
}

QStringList MYOProxy::initServerList()
{
    QStringList ret;
    ret << TLSServerListener::ID;
    ret << UdpServerListener::ID;
    ret << FileBlockSource::ID;
    return ret;
}

QStringList MYOProxy::initClientList()
{
    QStringList ret;
    ret << TLSClientListener::ID;
    ret << UdpClientListener::ID;
    ret << PipeClientListener::ID;
    ret << SharedMemorySource::ID;
    ret << FileBlockSource::ID;
    return ret;
}

MYOProxy::CLIENTS MYOProxy::getClientType() const
{
    return clientType;
}

MYOProxy::SERVERS MYOProxy::getServerType() const
{
    return serverType;
}
