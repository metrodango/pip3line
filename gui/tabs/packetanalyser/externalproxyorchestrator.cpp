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

        connect(this, &ExternalProxyOrchestrator::startChildren, inboundSource, &BlocksSource::startListening, Qt::QueuedConnection);
        connect(this, &ExternalProxyOrchestrator::stopChildren, inboundSource, &BlocksSource::stopListening, Qt::QueuedConnection);
        connect(this, &ExternalProxyOrchestrator::resetChildren, inboundSource, &BlocksSource::restart, Qt::QueuedConnection);
        connect(inboundSource, &BlocksSource::started, this, &ExternalProxyOrchestrator::started);
        connect(inboundSource, &BlocksSource::stopped, this, &ExternalProxyOrchestrator::stopped);
        connect(inboundSource, &BlocksSource::destroyed, [=](QObject *) { onInboundBlockSourceDestroyed();});
        connect(inboundSource, &BlocksSource::blockReceived, this, &ExternalProxyOrchestrator::onBlockReceived);
        connect(inboundSource, &BlocksSource::log, this, &ExternalProxyOrchestrator::log, Qt::QueuedConnection);
        connect(inboundSource, &BlocksSource::updated, this, &ExternalProxyOrchestrator::connectionsChanged, Qt::QueuedConnection);
        connect(inboundSource, &BlocksSource::newConnection, this, [=](BlocksSource *) { SourcesOrchestatorAbstract::connectionsChanged();}, Qt::QueuedConnection);
        connect(inboundSource, &BlocksSource::reflexionChanged, this, &ExternalProxyOrchestrator::checkForwarder, Qt::QueuedConnection);

        connect(this, &ExternalProxyOrchestrator::startChildren, outboundSource, &BlocksSource::startListening, Qt::QueuedConnection);
        connect(this, &ExternalProxyOrchestrator::stopChildren, outboundSource, &BlocksSource::stopListening, Qt::QueuedConnection);
        connect(this, &ExternalProxyOrchestrator::resetChildren, outboundSource, &BlocksSource::restart, Qt::QueuedConnection);
        connect(outboundSource, &BlocksSource::started, this, &ExternalProxyOrchestrator::started);
        connect(outboundSource, &BlocksSource::stopped, this, &ExternalProxyOrchestrator::stopped);
        connect(outboundSource, &BlocksSource::destroyed, this, &ExternalProxyOrchestrator::onOutboundBlockSourceDestroyed);
        connect(outboundSource, &BlocksSource::blockReceived, this, &ExternalProxyOrchestrator::onBlockReceived);
        connect(outboundSource, &BlocksSource::log, this, &ExternalProxyOrchestrator::log, Qt::QueuedConnection);
        connect(outboundSource, &BlocksSource::updated, this, &ExternalProxyOrchestrator::connectionsChanged, Qt::QueuedConnection);
        connect(outboundSource, &BlocksSource::newConnection, this, &ExternalProxyOrchestrator::connectionsChanged, Qt::QueuedConnection);
        connect(outboundSource, &BlocksSource::reflexionChanged, this, &ExternalProxyOrchestrator::checkForwarder, Qt::QueuedConnection);
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

void ExternalProxyOrchestrator::postPacket(QSharedPointer<Packet> packet)
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

        QSharedPointer<Packet> pac = QSharedPointer<Packet> (new(std::nothrow) Packet(block));
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
