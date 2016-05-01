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
        connect(this, SIGNAL(startChildren()), serverSource, SLOT(startListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(stopChildren()), serverSource, SLOT(stopListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(resetChildren()), serverSource, SLOT(restart()), Qt::QueuedConnection);
        connect(serverSource, SIGNAL(started()), SIGNAL(started()));
        connect(serverSource, SIGNAL(stopped()), SIGNAL(stopped()));
        connect(serverSource, SIGNAL(destroyed(QObject*)), this, SLOT(onServerSourceDestroyed()));
        connect(serverSource, SIGNAL(blockReceived(Block*)), SLOT(onBlockReceived(Block*)));
        connect(serverSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
        connect(serverSource, SIGNAL(updated()), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(serverSource, SIGNAL(newConnection(BlocksSource*)), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(serverSource, SIGNAL(connectionClosed(BlocksSource*)), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
    } else {
        qCritical() << tr("[ProxyOrchestrator::ProxyOrchestrator] server source is null T_T");
    }

    if (clientSource != nullptr) {

        connect(this, SIGNAL(startChildren()), clientSource, SLOT(startListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(stopChildren()), clientSource, SLOT(stopListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(resetChildren()), clientSource, SLOT(restart()), Qt::QueuedConnection);
        connect(clientSource, SIGNAL(started()), SIGNAL(started()));
        connect(clientSource, SIGNAL(stopped()), SIGNAL(stopped()));
        connect(clientSource, SIGNAL(destroyed(QObject*)), this, SLOT(onClientSourceDestroyed()));
        connect(clientSource, SIGNAL(blockReceived(Block*)), SLOT(onBlockReceived(Block*)));
        connect(clientSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
        connect(clientSource, SIGNAL(updated()), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(clientSource, SIGNAL(newConnection(BlocksSource*)), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(clientSource, SIGNAL(connectionClosed(BlocksSource*)), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
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

void ProxyOrchestrator::postPacket(Packet *packet)
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
    } else { // NODIRECTION
        if (source == serverSource) {
            serverSource->postBlockForSending(packet->toBlock());
        } else if (source == clientSource) {
            clientSource->postBlockForSending(packet->toBlock());
        } else {
            qCritical() << tr("[ProxyOrchestrator::postPacket] Cannot recognized the source, dropping packet T_T");
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

        Packet * pac = new(std::nothrow) Packet(block);
        if (pac == nullptr) {
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

