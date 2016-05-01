#include "sourcesorchestatorabstract.h"
#include "sources/blocksources/blockssource.h"
#include "shared/defaultcontrolgui.h"
#include <QHBoxLayout>
#include <QPushButton>
#include "packet.h"
#include <QDebug>

const QStringList SourcesOrchestatorAbstract::OrchestratorsList = SourcesOrchestatorAbstract::initSourcesStrings();

SourcesOrchestatorAbstract::SourcesOrchestatorAbstract(QObject *parent) :
    QObject(parent),
    hasDirection(false)
{
    confgui = nullptr;
    controlGui = nullptr;
    forwarder = false;
    type = INVALID_TYPE;
}

SourcesOrchestatorAbstract::~SourcesOrchestatorAbstract()
{
    qDebug() << "destroying" << this;
    delete confgui;
    delete controlGui;
}

bool SourcesOrchestatorAbstract::isForwarder() const
{
    return forwarder;
}

void SourcesOrchestatorAbstract::sourceReflexionChanged(bool value)
{
    if (forwarder != value) {
        forwarder = value;
        emit forwardingChanged(forwarder);
    }
}

QWidget *SourcesOrchestatorAbstract::requestControlGui(QWidget * parent)
{
    DefaultControlGui * wid = new(std::nothrow) DefaultControlGui(parent);
    if (wid == nullptr) {
        qFatal("Cannot allocate memory for DefaultControlGui X{");
    }

    connect(wid, SIGNAL(start()), this, SLOT(start()),Qt::QueuedConnection);
    connect(wid, SIGNAL(stop()), this, SLOT(stop()),Qt::QueuedConnection);
    connect(wid, SIGNAL(reset()), this, SLOT(restart()),Qt::QueuedConnection);
    connect(this, SIGNAL(started()), wid, SLOT(receiveStart()), Qt::QueuedConnection);
    connect(this, SIGNAL(stopped()), wid, SLOT(receiveStop()), Qt::QueuedConnection);
    connect(wid, SIGNAL(requestConfPanel()), this, SIGNAL(guiRequested()));

    return wid;
}

QWidget *SourcesOrchestatorAbstract::requestConfGui(QWidget * parent)
{
    // setting up a default widget with a horizontal layout
    QWidget *wid = new(std::nothrow) QWidget(parent);
    if (wid == nullptr) {
        qFatal("Cannot allocate memory for QWidget X{");
    }

    QHBoxLayout * layout = new(std::nothrow) QHBoxLayout(parent);
    if (layout == nullptr) {
        qFatal("Cannot allocate memory for QHBoxLayout X{");
    }
    wid->setLayout(layout);

    return wid;
}
bool SourcesOrchestatorAbstract::getHasDirection() const
{
    return hasDirection;
}

QHash<QString, QString> SourcesOrchestatorAbstract::getConfiguration()
{
    QHash<QString, QString> ret;

    return ret;
}

void SourcesOrchestatorAbstract::setConfiguration(QHash<QString, QString> conf)
{
    qDebug() << "Configuration size" << conf.size();
}

void SourcesOrchestatorAbstract::postPacket(Packet *packet)
{
    if (packet->isInjected())
        emit packetInjected(packet);
}

void SourcesOrchestatorAbstract::setType(const OrchestratorType &value)
{
    type = value;
    if (type != INVALID_TYPE)
        setName(SourcesOrchestatorAbstract::OrchestratorsList.at(type - 1));
}

QWidget *SourcesOrchestatorAbstract::getControlGui(QWidget *parent)
{
    if (controlGui == nullptr) {
        controlGui = requestControlGui(parent);
        if (controlGui != nullptr)
            connect(controlGui, SIGNAL(destroyed(QObject*)), this, SLOT(onControlGuiDestroyed()));
    }
    return controlGui;
}

QWidget *SourcesOrchestatorAbstract::getConfGui(QWidget *parent)
{
    if (confgui == nullptr) {
        confgui = requestConfGui(parent);
        if (confgui != nullptr)
            connect(confgui, SIGNAL(destroyed(QObject*)), this, SLOT(onConfigGuiDestroyed()));
    }

    return confgui;
}

Target<SourcesOrchestatorAbstract *> SourcesOrchestatorAbstract::toOrchestratorTarget(Target<BlocksSource *> bst)
{
    Target<SourcesOrchestatorAbstract *> ret;
    ret.setConnectionID(bst.getConnectionID());
    ret.setDescription(bst.getDescription());
    ret.setTimestamp(bst.getTimestamp());
    ret.setSource(this);

    return ret;
}

QList<Target<SourcesOrchestatorAbstract *> > SourcesOrchestatorAbstract::toOrchestratorTargetList(QList<Target<BlocksSource *> > ilist)
{
    QList<Target<SourcesOrchestatorAbstract *> > list;
    for (int i = 0 ; i < ilist.size(); i++) {
        list.append(toOrchestratorTarget(ilist.at(i)));
    }

    return list;
}

bool SourcesOrchestatorAbstract::start()
{
    emit startChildren();
    return true;
}

void SourcesOrchestatorAbstract::stop()
{
    emit stopChildren();
}

bool SourcesOrchestatorAbstract::restart()
{
    emit resetChildren();
    return true;
}

void SourcesOrchestatorAbstract::onBlockReceived(Block *block)
{
    Packet * pac = new(std::nothrow) Packet(block);
    if (pac == nullptr) {
        qFatal("Cannot allocate Packet X{");
    }

    pac->setDirection(Packet::RIGHTLEFT);

    delete block;
    emit newPacket(pac);
}

SourcesOrchestatorAbstract::OrchestratorType SourcesOrchestatorAbstract::getType() const
{
    return type;
}

QStringList SourcesOrchestatorAbstract::initSourcesStrings()
{
    QStringList list;

    list << "TCP client"
         << "UDP Client"
         << "TCP server"
         << "UDP server"
         << "TCP proxy"
         << "UDP proxy"
         << "External Proxy (TCP)"
         << "External Proxy (UDP)";

    return list;
}

QString SourcesOrchestatorAbstract::getDescription() const
{
    return description;
}

void SourcesOrchestatorAbstract::setDescription(const QString &value)
{
    description = value;
}

void SourcesOrchestatorAbstract::onControlGuiDestroyed()
{
    controlGui = nullptr;
}

void SourcesOrchestatorAbstract::onConfigGuiDestroyed()
{
    confgui = nullptr;
}

QString SourcesOrchestatorAbstract::getName() const
{
    return name;
}

void SourcesOrchestatorAbstract::setName(const QString &value)
{
    name = value;
}


