#include "proxyorchestrator.h"
#include "packet.h"
#include <QDebug>

ProxyOrchestrator::ProxyOrchestrator(BlocksSource *serverSource, BlocksSource *clientSource, QObject *parent) :
    SourcesOrchestatorAbstract(parent),
    serverSource(serverSource),
    clientSource(clientSource)
{
    hasDirection = true;
    forwarder = true;

    if (serverSource != nullptr) {
        connect(this, &ProxyOrchestrator::startChildren, serverSource, &BlocksSource::startListening, Qt::QueuedConnection);
        connect(this, &ProxyOrchestrator::stopChildren, serverSource, &BlocksSource::stopListening, Qt::QueuedConnection);
        connect(this, &ProxyOrchestrator::resetChildren, serverSource, &BlocksSource::restart, Qt::QueuedConnection);
        connect(serverSource, &BlocksSource::started, this, &ProxyOrchestrator::started);
        connect(serverSource, &BlocksSource::stopped, this, &ProxyOrchestrator::stopped);
        connect(serverSource, &BlocksSource::destroyed, this, &ProxyOrchestrator::onServerSourceDestroyed);
        connect(serverSource, &BlocksSource::blockReceived, this, &ProxyOrchestrator::onBlockReceived);
        connect(serverSource, &BlocksSource::log, this, &ProxyOrchestrator::log, Qt::QueuedConnection);
        connect(serverSource, &BlocksSource::updated, this, &ProxyOrchestrator::connectionsChanged, Qt::QueuedConnection);
        connect(serverSource, &BlocksSource::newConnection, this, &ProxyOrchestrator::connectionsChanged, Qt::QueuedConnection);
        connect(serverSource, &BlocksSource::connectionClosed, clientSource, &BlocksSource::onConnectionClosed, Qt::QueuedConnection);
    } else {
        qCritical() << tr("[ProxyOrchestrator::ProxyOrchestrator] server source is null T_T");
    }

    if (clientSource != nullptr) {

        connect(this, &ProxyOrchestrator::startChildren, clientSource, &BlocksSource::startListening, Qt::QueuedConnection);
        connect(this, &ProxyOrchestrator::stopChildren, clientSource, &BlocksSource::stopListening, Qt::QueuedConnection);
        connect(this, &ProxyOrchestrator::resetChildren, clientSource, &BlocksSource::restart, Qt::QueuedConnection);
        connect(clientSource, &BlocksSource::started, this, &ProxyOrchestrator::started);
        connect(clientSource, &BlocksSource::stopped, this, &ProxyOrchestrator::stopped);
        connect(clientSource, &BlocksSource::destroyed, this, &ProxyOrchestrator::onClientSourceDestroyed);
        connect(clientSource, &BlocksSource::blockReceived, this, &ProxyOrchestrator::onBlockReceived);
        connect(clientSource, &BlocksSource::log, this, &ProxyOrchestrator::log, Qt::QueuedConnection);
        connect(clientSource, &BlocksSource::updated, this, &ProxyOrchestrator::connectionsChanged, Qt::QueuedConnection);
        connect(clientSource, &BlocksSource::newConnection, this, &ProxyOrchestrator::connectionsChanged, Qt::QueuedConnection);
        connect(clientSource, &BlocksSource::connectionClosed, serverSource, &BlocksSource::onConnectionClosed, Qt::QueuedConnection);
    } else {
        qCritical() << tr("[ProxyOrchestrator::ProxyOrchestrator] client source is null T_T");
    }
}

ProxyOrchestrator::~ProxyOrchestrator()
{
    delete serverSource;
    delete clientSource;
}

QList<Target<SourcesOrchestatorAbstract *> > ProxyOrchestrator::getAvailableConnections()
{
    QList<Target<SourcesOrchestatorAbstract *> >  list;
    if (serverSource != nullptr) {
        list.append(toOrchestratorTargetList(serverSource->getAvailableConnections()));
    }

    if (clientSource != nullptr) {
        list.append(toOrchestratorTargetList(clientSource->getAvailableConnections()));
    }

    return list;
}

BlocksSource *ProxyOrchestrator::getBlockSource(int index)
{
    if (index == 0) {
        return serverSource;
    } else if (index == 1) {
        return clientSource;
    }

    return nullptr;
}

int ProxyOrchestrator::blockSourceCount() const
{
    return 2;
}

void ProxyOrchestrator::postPacket(QSharedPointer<Packet> packet)
{
    if (serverSource == nullptr || clientSource == nullptr) {
        qCritical() << tr("[ProxyOrchestrator::postPacket] One of the source is null T_T");
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

bool ProxyOrchestrator::start()
{
    if (serverSource == nullptr || clientSource == nullptr ) {
        return false;
    }

    emit startChildren();

    return true;
}

void ProxyOrchestrator::onBlockReceived(Block *block)
{
    BlocksSource * senderObj = dynamic_cast<BlocksSource *>(sender());
    if (senderObj != nullptr) {
        if (senderObj != serverSource && senderObj != clientSource) {
            qCritical() << tr("[ProxyOrchestrator::onBlockReceived] Cannot recognized the sender, dropping the block T_T");
            delete block;
            return;
        }

        QSharedPointer<Packet> pac = QSharedPointer<Packet> (new(std::nothrow) Packet(block));
        if (pac.isNull()) {
            qFatal("Cannot allocate Packet X{");
        }

        if (senderObj == serverSource) {
            pac->setDirection(Packet::LEFTRIGHT);
        } else {
            pac->setDirection(Packet::RIGHTLEFT);
        }

        emit newPacket(pac);
        delete block;
    } else {
        qCritical() << tr("[ProxyOrchestrator::onBlockReceived] Cannot cast the sender T_T");
    }
}

void ProxyOrchestrator::onServerSourceDestroyed()
{
    serverSource = nullptr;
}

void ProxyOrchestrator::onClientSourceDestroyed()
{
    clientSource = nullptr;
}

