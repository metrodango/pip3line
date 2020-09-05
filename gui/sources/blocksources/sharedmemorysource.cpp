#include "sharedmemorysource.h"
#include "shared/guiconst.h"
#include "shmtypes/sharedmemorysourcebasewidget.h"
#include "shmtypes/sharedmemconnector.h"
#include "shmtypes/sharedmemposix.h"
#ifdef Q_OS_UNIX
#include "shmtypes/sysv_shm.h"
#endif

const QString SharedMemorySource::ID = QString("SharedMemory");

SharedMemorySource::SharedMemorySource(QObject *parent) : BlocksSource(parent)
{
    running = false;
    sid = BlocksSource::newSourceID(this);

    memConn = nullptr;
    localGui = nullptr;

    flags = WANTS_TRACKING;
    type = BlocksSource::CLIENT;
    timerInterval = 200;
    checkTimer.setSingleShot(false);
    checkTimer.setInterval(timerInterval);

    checkTimer.moveToThread(&workerThread);
    connect(&checkTimer, &QTimer::timeout, this, &SharedMemorySource::checkData);
    connect(this, &SharedMemorySource::stopped, &checkTimer, &QTimer::stop, Qt::QueuedConnection);

    updateConnectionsTimer.moveToThread(&workerThread);
    moveToThread(&workerThread);
    workerThread.start();
}

SharedMemorySource::~SharedMemorySource()
{
    checkTimer.stop();
    workerThread.quit();
    workerThread.wait();

    BlocksSource::releaseID(sid);
    delete memConn;
}

QWidget *SharedMemorySource::getAdditionnalCtrls(QWidget *)
{
    return nullptr;
}

QString SharedMemorySource::getName()
{
    return ID;
}

QString SharedMemorySource::getDescription()
{
    return QString("Listener for Shared Memory");
}

bool SharedMemorySource::isStarted()
{
    return running;
}

QHash<QString, QString> SharedMemorySource::getConfiguration()
{
    QHash<QString, QString> ret = BlocksSource::getConfiguration();
    if (memConn != nullptr) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        ret.insert(memConn->getConfiguration());
#else
        ret.unite(memConn->getConfiguration());
#endif

    }
    return ret;
}

void SharedMemorySource::setConfiguration(const QHash<QString, QString> &conf)
{
    BlocksSource::setConfiguration(conf);
    if (conf.contains(GuiConst::STATE_TYPE)) {
        bool ok = false;
        int val = conf.value(GuiConst::STATE_TYPE).toInt(&ok);
        if (ok) {
            setShmType(val);
        }
    }

    if (memConn != nullptr) {
        memConn->setConfiguration(conf);
    }
}

QStringList SharedMemorySource::getAvailableSHMTypes()
{
    QStringList ret;
    ret << SharedMemPosix::ID;
#ifdef Q_OS_UNIX
    ret << SysV_Shm::ID;
#endif
    return ret;
}

void SharedMemorySource::setShmType(int smtype)
{
    SharedmemConnector * oldm = memConn;
    memConn = nullptr;
    switch (smtype) {
        case SharedmemConnector::POSIX_NATIVE_SHM:
            memConn = new(std::nothrow) SharedMemPosix();
            if (memConn == nullptr) {
                qFatal("%s", GuiConst::MEM_ALLOC_ERROR_STR);
            }
            shmType = smtype;
            break;
        case SharedmemConnector::SYSV_SHM:
#ifdef Q_OS_UNIX
            memConn = new(std::nothrow) SysV_Shm();
            if (memConn == nullptr) {
                qFatal("%s", GuiConst::MEM_ALLOC_ERROR_STR);
            }
            shmType = smtype;
            break;
#else
            qCritical() << tr("Ssytem V shm unsupported on this platform");
#endif
        default:
            qCritical() << tr("[SharedMemorySource::setType] Unknown type : %1");
            break;
    }

    if (memConn == nullptr) {
        memConn = oldm;
    } else {
        if (localGui != nullptr) {
            localGui->addWidget(memConn->getGui());
        }
        memConn->moveToThread(&workerThread);
        connect(memConn, &SharedmemConnector::reset, this, &SharedMemorySource::onShmReset, Qt::QueuedConnection);
        connect(memConn, &SharedmemConnector::log, this , &SharedMemorySource::log, Qt::QueuedConnection);
        delete oldm;
    }
}

int SharedMemorySource::getShmType() const
{
    return shmType;
}

void SharedMemorySource::sendBlock(Block *block)
{
    if (running && memConn != nullptr) {

        int sid = block->getSourceid();
        if (!extsources.contains(sid)) {
            extsources.append(sid);
        }
        memConn->writeData(block->getData());
        currentData = block->getData();
    } else {
        qCritical() << tr("[SharedMemorySource::sendBlock] Not running and/or shared mem obj is null");
    }

    delete block;
}

void SharedMemorySource::checkData()
{

    if (running && memConn != nullptr) {
        QByteArray data;
        if (memConn->readData(data)) {
            if (currentData != data) {
                currentData = data;
                Block * bl = new(std::nothrow) Block(data,Block::INVALID_ID);
                if (bl == nullptr) {
                    qFatal("Cannot allocate memory");
                }
                emit blockReceived(bl);
//                for (int i = 0; i < extsources.size(); i++) {
//                    int bid = extsources.at(i);
//                    Block * bl = new(std::nothrow) Block(data,bid);
//                    if (bl == nullptr) {
//                        qFatal("Cannot allocate memory");
//                    }
//                    emit blockReceived(bl);
//                }
            }
        }
    }
}

void SharedMemorySource::onShmReset()
{
    stopListening();
    startListening();
}

bool SharedMemorySource::startListening()
{
    if (!running) {
        currentData.clear();
        if (memConn != nullptr) {
            running = memConn->connectToMem();
            if (running)  {
                checkTimer.start(timerInterval);
                emit started();
                updateConnectionsInfo();
                emit log("Started monitoring", memConn->name(),Pip3lineConst::PLSTATUS);
            }
        } else {
            qCritical() << tr("[SharedMemorySource::startListening] Shared memory object is null");
        }
    }

    return running;
}

void SharedMemorySource::stopListening()
{
    if (running) {

        if (memConn != nullptr) {
            memConn->disconnectFromMem();
            emit log("Stopped monitoring", memConn->name(),Pip3lineConst::PLSTATUS);
        } else {
            qCritical() << tr("[SharedMemorySource::stopListening] Shared memory object is null");
        }
        running = false;

        currentData.clear();
        emit stopped();
        triggerUpdate();
    }
}

void SharedMemorySource::onGuiDestroyed()
{
    localGui = nullptr;
}

void SharedMemorySource::onConnectionClosed(int cid)
{
    if (!extsources.contains(cid)) {
        extsources.removeAll(cid);
    }
}

QWidget *SharedMemorySource::requestGui(QWidget *parent)
{
    if (localGui == nullptr) {
        localGui = new(std::nothrow) SharedMemorySourceBaseWidget(this, parent);
        connect(localGui, &SharedMemorySourceBaseWidget::destroyed, this , &SharedMemorySource::onGuiDestroyed);
        if (localGui == nullptr) {
            qFatal("Cannot allocate memory");
        }
    }

    if (memConn != nullptr) {
        localGui->addWidget(memConn->getGui());
    }

    return localGui;
}

void SharedMemorySource::internalUpdateConnectionsInfo()
{
    connectionsInfo.clear();

    if (running) { // accepting new connections
        Target<BlocksSource *> tac;
        QString desc = memConn->name();
        if (memConn != nullptr) {
            desc.append(": ").append(memConn->getCurrentID());
        } else {
            desc.append(": Unitialized");
        }

        tac.setDescription(desc);
        tac.setConnectionID(sid);
        tac.setSource(this);
        connectionsInfo.append(tac);
    }
}
