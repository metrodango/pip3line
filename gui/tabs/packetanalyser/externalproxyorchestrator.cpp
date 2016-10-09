#include "externalproxyorchestrator.h"
#include "packet.h"
#include <QDebug>

ExternalProxyOrchestrator::ExternalProxyOrchestrator(BlocksSource *inboundSource, BlocksSource *outboundSource, QObject *parent) :
    SourcesOrchestatorAbstract(parent),
    inboundSource(inboundSource),
    outboundSource(outboundSource)
{
    hasDirection = true;
    if (inboundSource != nullptr && outboundSource != nullptr) {
        checkForwarder();

        connect(this, SIGNAL(startChildren()), inboundSource, SLOT(startListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(stopChildren()), inboundSource, SLOT(stopListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(resetChildren()), inboundSource, SLOT(restart()), Qt::QueuedConnection);
        connect(inboundSource, SIGNAL(started()), SIGNAL(started()));
        connect(inboundSource, SIGNAL(stopped()), SIGNAL(stopped()));
        connect(inboundSource, SIGNAL(destroyed(QObject*)), this, SLOT(onInboundBlockSourceDestroyed()));
        connect(inboundSource, SIGNAL(blockReceived(Block*)), SLOT(onBlockReceived(Block*)));
        connect(inboundSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
        connect(inboundSource, SIGNAL(updated()), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(inboundSource, SIGNAL(newConnection(BlocksSource*)), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(inboundSource, SIGNAL(reflexionChanged(bool)), this, SLOT(checkForwarder()), Qt::QueuedConnection);

        connect(this, SIGNAL(startChildren()), outboundSource, SLOT(startListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(stopChildren()), outboundSource, SLOT(stopListening()), Qt::QueuedConnection);
        connect(this, SIGNAL(resetChildren()), outboundSource, SLOT(restart()), Qt::QueuedConnection);
        connect(outboundSource, SIGNAL(started()), SIGNAL(started()));
        connect(outboundSource, SIGNAL(stopped()), SIGNAL(stopped()));
        connect(outboundSource, SIGNAL(destroyed(QObject*)), this, SLOT(onOutboundBlockSourceDestroyed()));
        connect(outboundSource, SIGNAL(blockReceived(Block*)), SLOT(onBlockReceived(Block*)));
        connect(outboundSource, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
        connect(outboundSource, SIGNAL(updated()), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(outboundSource, SIGNAL(newConnection(BlocksSource*)), this, SIGNAL(connectionsChanged()), Qt::QueuedConnection);
        connect(outboundSource, SIGNAL(reflexionChanged(bool)), this, SLOT(checkForwarder()), Qt::QueuedConnection);
    } else {
        if (inboundSource == nullptr) {
            qCritical() << tr("[ExternalProxyUDPOrchestrator::ExternalProxyUDPOrchestrator] inbound source is null T_T");
        } else {
            qCritical() << tr("[ExternalProxyUDPOrchestrator::ExternalProxyUDPOrchestrator] outbound source is null T_T");
        }
    }
}

ExternalProxyOrchestrator::~ExternalProxyOrchestrator()
{
    delete inboundSource;
    delete outboundSource;
}

QList<Target<SourcesOrchestatorAbstract *> > ExternalProxyOrchestrator::getAvailableConnections()
{
    QList<Target<SourcesOrchestatorAbstract *> >  list;
    if (inboundSource != nullptr) {
        list.append(toOrchestratorTargetList(inboundSource->getAvailableConnections()));
    }

    if (outboundSource != nullptr) {
        list.append(toOrchestratorTargetList(outboundSource->getAvailableConnections()));
    }

    return list;
}

BlocksSource *ExternalProxyOrchestrator::getBlockSource(int index)
{
    if (index == 0)
        return inboundSource;

    if (index == 1)
        return outboundSource;

    return nullptr;
}

int ExternalProxyOrchestrator::blockSourceCount() const
{
    return ((inboundSource != nullptr ? 1 : 0) + (outboundSource != nullptr ? 1 : 0));
}

void ExternalProxyOrchestrator::postPacket(Packet *packet)
{

    SourcesOrchestatorAbstract::postPacket(packet);
    BlocksSource * source = BlocksSource::getSourceObject(packet->getSourceid());
    if (source == inboundSource && inboundSource != nullptr) {
        packet->setDirection(Packet::RIGHTLEFT);
        inboundSource->postBlockForSending(packet->toBlock());
    } else if (source == outboundSource && outboundSource != nullptr) {
        packet->setDirection(Packet::LEFTRIGHT);
        outboundSource->postBlockForSending(packet->toBlock());
    } else {
        emit log(tr("Connection [%1] cannot be identified, the packet will not be send").arg(packet->getSourceid()),
                 metaObject()->className(),
                 Pip3lineConst::LERROR);
    }
}

bool ExternalProxyOrchestrator::start()
{
    if (inboundSource == nullptr || outboundSource == nullptr ) {
        return false;
    }

    emit startChildren();

    return true;

}

void ExternalProxyOrchestrator::onBlockReceived(Block *block)
{
    BlocksSource * senderObj = dynamic_cast<BlocksSource *>(sender());
    if (senderObj != nullptr) {
        if (senderObj != inboundSource && senderObj != outboundSource) {
            qCritical() << tr("[ExternalProxyUDPOrchestrator::onBlockReceived] Cannot recognized the sender, dropiing block T_T");
            delete block;
            return;
        }

        Packet * pac = new(std::nothrow) Packet(block);
        if (pac == nullptr) {
            qFatal("Cannot allocate Packet X{");
        }

        if (senderObj == inboundSource) {
            pac->setDirection(Packet::LEFTRIGHT);
        } else {
            pac->setDirection(Packet::RIGHTLEFT);
        }

        emit newPacket(pac);
        delete block;
    } else {
        qCritical() << tr("[ExternalProxyUDPOrchestrator::onBlockReceived] Error while casting the sender T_T");
    }
}

void ExternalProxyOrchestrator::onInboundBlockSourceDestroyed()
{
    inboundSource = nullptr;
}

void ExternalProxyOrchestrator::onOutboundBlockSourceDestroyed()
{
    outboundSource = nullptr;
}

void ExternalProxyOrchestrator::checkForwarder()
{
    if (inboundSource != nullptr && outboundSource != nullptr) {
        bool temp = false;
        if (inboundSource->isReflexive() && outboundSource->isReflexive()) {
            temp = true;
        }

        if (forwarder != temp) {
            forwarder = temp;
            emit forwardingChanged(forwarder);
        }
    }
}

Target<SourcesOrchestatorAbstract *> ExternalProxyOrchestrator::toOrchestratorTarget(Target<BlocksSource *> bst)
{
    QString addedString;
    if (bst.getSource() == inboundSource) {
        addedString = " { Inbound }";
    } else {
        addedString = " { Outbound }";
    }
    Target<SourcesOrchestatorAbstract *> ta = SourcesOrchestatorAbstract::toOrchestratorTarget(bst);
    ta.setDescription(bst.getDescription().append(addedString));
    return ta;
}
