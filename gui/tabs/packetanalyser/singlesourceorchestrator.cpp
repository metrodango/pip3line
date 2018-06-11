#include "singlesourceorchestrator.h"
#include "sources/blocksources/blockssource.h"
#include "packet.h"
#include <QDebug>
#include <shared/defaultcontrolgui.h>

SingleSourceOrchestrator::SingleSourceOrchestrator(BlocksSource *source, QObject *parent) :
    SourcesOrchestatorAbstract(parent),
    source(source)
{
    hasDirection = true;
    if (source != nullptr) {

        forwarder = source->isReflexive();

        connect(this, &SourcesOrchestatorAbstract::startChildren, source, &BlocksSource::startListening, Qt::QueuedConnection);
        connect(this, &SourcesOrchestatorAbstract::stopChildren, source, &BlocksSource::stopListening, Qt::QueuedConnection);
        connect(this, &SourcesOrchestatorAbstract::resetChildren, source, &BlocksSource::restart, Qt::QueuedConnection);
        connect(source, &BlocksSource::started, this, &SourcesOrchestatorAbstract::started);
        connect(source, &BlocksSource::stopped, this, &SourcesOrchestatorAbstract::stopped);
        connect(source, &BlocksSource::destroyed, this, &SingleSourceOrchestrator::onBlockSourceDestroyed);
        connect(source, &BlocksSource::blockReceived, this, &SingleSourceOrchestrator::onBlockReceived);
        connect(source, &BlocksSource::reflexionChanged, this, &SingleSourceOrchestrator::sourceReflexionChanged);
        connect(source, &BlocksSource::log, this, &SourcesOrchestatorAbstract::log, Qt::QueuedConnection);
        connect(source, &BlocksSource::updated, this, &SourcesOrchestatorAbstract::connectionsChanged, Qt::QueuedConnection);
        connect(source, &BlocksSource::newConnection, this, &SourcesOrchestatorAbstract::connectionsChanged, Qt::QueuedConnection);
    }
}

SingleSourceOrchestrator::~SingleSourceOrchestrator()
{
    disconnect(source, &BlocksSource::destroyed, this, &SingleSourceOrchestrator::onBlockSourceDestroyed);
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

void SingleSourceOrchestrator::postPacket(QSharedPointer<Packet> packet)
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

QWidget *SingleSourceOrchestrator::requestControlGui(QWidget *parent)
{
    QWidget * defCtrlWidget = SourcesOrchestatorAbstract::requestControlGui(parent);
    if (defCtrlWidget != nullptr) {
        QWidget *addCtrl = source->getAdditionnalCtrls();
        if (addCtrl != nullptr) {
            DefaultControlGui * defCtrlGui = static_cast<DefaultControlGui *>(defCtrlWidget);
            defCtrlGui->appendWidget(addCtrl);
        }
    }

    return defCtrlWidget;
}

