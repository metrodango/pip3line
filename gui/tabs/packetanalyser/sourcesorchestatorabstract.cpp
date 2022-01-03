#include "sourcesorchestatorabstract.h"
#include "sources/blocksources/blockssource.h"
#include "shared/defaultcontrolgui.h"
#include <QHBoxLayout>
#include <QPushButton>
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

void SourcesOrchestatorAbstract::sourceReflexionChanged(const bool &value)
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

    connect(wid, &DefaultControlGui::start, this, &SourcesOrchestatorAbstract::start,Qt::QueuedConnection);
    connect(wid, &DefaultControlGui::stop, this, &SourcesOrchestatorAbstract::stop,Qt::QueuedConnection);
    connect(wid, &DefaultControlGui::reset, this, &SourcesOrchestatorAbstract::restart,Qt::QueuedConnection);
    connect(this, &SourcesOrchestatorAbstract::started, wid, &DefaultControlGui::receiveStart, Qt::QueuedConnection);
    connect(this, &SourcesOrchestatorAbstract::stopped, wid, &DefaultControlGui::receiveStop, Qt::QueuedConnection);
    connect(wid, &DefaultControlGui::requestConfPanel, this, &SourcesOrchestatorAbstract::guiRequested);

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

    int count = blockSourceCount();
    for (int i = 0; i < count; i++) {
        BlocksSource * bs = getBlockSource(i);
        if (bs != nullptr) {
            QWidget * wid = bs->getGui();
            if (wid != nullptr) {
                layout->addWidget(wid);
            }
        } else {
            qCritical() << tr("[OrchestratorChooser::setupOrchestrator] BlockSource is nullptr T_T");
        }
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

bool SourcesOrchestatorAbstract::isTrackingChanges()
{
    return false;
}

void SourcesOrchestatorAbstract::postPacket(QSharedPointer<Packet> packet)
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
            connect(controlGui, &QWidget::destroyed, this, &SourcesOrchestatorAbstract::onControlGuiDestroyed);
    }
    return controlGui;
}

QWidget *SourcesOrchestatorAbstract::getConfGui(QWidget *parent)
{
    if (confgui == nullptr) {
        confgui = requestConfGui(parent);
        if (confgui != nullptr)
            connect(confgui, &QWidget::destroyed, this, &SourcesOrchestatorAbstract::onConfigGuiDestroyed);
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
        Target<SourcesOrchestatorAbstract *> ntarget = toOrchestratorTarget(ilist.at(i));
        list.append(ntarget);
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
    QSharedPointer<Packet> pac = QSharedPointer<Packet> (new(std::nothrow) Packet(block));
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
         << "External Proxy (UDP)"
         << "SOCKS 5 Proxy"
#if defined(Q_OS_WIN32)
         << "Named Pipe client"
#else
         << "UNIX Local socket client"
#endif
         << "MYO Proxy"
         << "Shared Memory"
         << "File"
         << "DTLS client"
         << "DTLS Server"
         << "DTLS Proxy";

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


