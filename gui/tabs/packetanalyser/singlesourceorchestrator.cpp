#include "singlesourceorchestrator.h"
#include "sources/blocksources/blockssource.h"
#include "packet.h"
#include <QDebug>

SingleSourceOrchestrator::SingleSourceOrchestrator(BlocksSource *source, QObject *parent) :
    SourcesOrchestatorAbstract(parent),
    source(source)
{
    hasDirection = true;
    if (source != nullptr) {

        forwarder = source->isReflexive();

        connect(this, SIGNAL(startChildren()), source, SLOT(startListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(stopChildren()), source, SLOT(stopListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(resetChildren()), source, SLOT(restart()), Qt::QueuedConnection);
        connect(source, SIGNAL(started()), SIGNAL(started()));
        connect(source, SIGNAL(stopped()), SIGNAL(stopped()));
        connect(source, SIGNAL(destroyed(QObject*)), this, SLOT(onBlockSourceDestroyed()));
        connect(source, SIGNAL(blockReceived(Block*)), SLOT(onBlockReceived(Block*)));
        connect(source, SIGNAL(reflexionChanged(bool)), this, SLOT(sourceReflexionChanged(bool)));
        connect(source, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), this, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
        connect(source, SIGNAL(updated()), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(source, SIGNAL(newConnection(BlocksSource*)), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
    }
}

SingleSourceOrchestrator::~SingleSourceOrchestrator()
{
    disconnect(source, SIGNAL(destroyed(QObject*)), this, SLOT(onBlockSourceDestroyed()));
    delete source;
}

QList<Target<SourcesOrchestatorAbstract *> > SingleSourceOrchestrator::getAvailableConnections()
{
    QList<Target<SourcesOrchestatorAbstract *> > ret;

    if (source != nullptr) {
        ret = toOrchestratorTargetList( source->getAvailableConnections());
    }

    return ret;
}

BlocksSource *SingleSourceOrchestrator::getBlockSource(int)
{ // there is only one block source, we don't care about the index
    return source;
}

int SingleSourceOrchestrator::blockSourceCount() const
{
    if(source != nullptr) {
        return 1;
    }
    return 0;
}

void SingleSourceOrchestrator::postPacket(Packet *packet)
{
    packet->setDirection(Packet::LEFTRIGHT);
    SourcesOrchestatorAbstract::postPacket(packet);
    if(source != nullptr) {
        source->postBlockForSending(packet->toBlock());
    }
}

bool SingleSourceOrchestrator::start()
{
    if (source != nullptr) {
        return SourcesOrchestatorAbstract::start();
    }

    return false;
}

void SingleSourceOrchestrator::onBlockSourceDestroyed()
{
    source = nullptr;
}

