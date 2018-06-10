#include "pipeclientlistener.h"
#include "pipeclientwidget.h"
#include <QDebug>
#include <QFileInfo>
#include <QPushButton>
#include "shared/guiconst.h"
#include <shared/newconnectionbutton.h>

#if defined(Q_OS_WIN32)
    const QString PipeClientListener::ID = QString("Named Pipe Client");
#else
    const QString PipeClientListener::ID = QString("Named Pipe Client");
#endif



PipeClientListener::PipeClientListener( QObject *parent) :
    BlocksSource(parent),
    running(false)
{
    pipeName = "InvalidPipeName";
    flags = REFLEXION_OPTIONS;
    type = CLIENT;
    updateTimer.moveToThread(&workerThread);
    moveToThread(&workerThread);
    workerThread.start();
}

PipeClientListener::~PipeClientListener()
{
    workerThread.quit();
    workerThread.wait();
    mapExtSourcesToLocal.clear();
}

QString PipeClientListener::getName()
{
    return ID;
}

QString PipeClientListener::getDescription()
{
#if defined(Q_OS_WIN32)
    QString desc = QObject::tr("Client for windows Named Pipe");
#else
    QString desc = QObject::tr("Client for Unix Local Sockets");
#endif
    return desc;
}

bool PipeClientListener::isStarted()
{
    return running;
}

QHash<QString, QString> PipeClientListener::getConfiguration()
{
    QHash<QString, QString> ret = BlocksSource::getConfiguration();
    ret.insert(GuiConst::STATE_NAME, QString::fromUtf8(pipeName.toUtf8().toBase64()));
    return ret;
}

void PipeClientListener::setConfiguration(QHash<QString, QString> conf)
{
    BlocksSource::setConfiguration(conf);
    if (conf.contains(GuiConst::STATE_NAME)) {
        QString path = QString::fromUtf8(QByteArray::fromBase64(conf.value(GuiConst::STATE_NAME).toUtf8()));
        if (!path.isEmpty()) {
            pipeName = path;
        } else {
            qCritical() << tr("[PipeClientListener::setConfiguration] Invalid Path, ignoring.");
        }
    }

}

QWidget *PipeClientListener::getAdditionnalCtrls(QWidget *parent)
{
    QPushButton * but = new(std::nothrow)NewConnectionButton (parent);
    if (but == nullptr) {
        qFatal("Cannot allocate memory for QPushButton X{");
    }
    connect(but, &QPushButton::clicked, this, &PipeClientListener::createConnection, Qt::QueuedConnection);
    return but;
}

void PipeClientListener::sendBlock(Block *block)
{
    if (running) {
        QByteArray data = applyOutboundTransform(block->getData());
        qint64 size = data.size();
        int sid = -1;
        if (mapExtSourcesToLocal.contains(block->getSourceid())) {
            sid = mapExtSourcesToLocal.value(block->getSourceid());
        }
        int bid = block->getSourceid();
        bool foundSource = false;

        QHashIterator<QLocalSocket *, PipeConnection> i(sockets);
        while (i.hasNext()) {
            i.next();
            int suid = i.value().sid;
            QLocalSocket *lsock = i.key();
            if (bid ==  suid || sid == suid) { // either we are sending directly to the blocksource or getting the block from another one
                qint64 bwritten = lsock->write(data);
                while (size > bwritten) {
                    bwritten += lsock->write(&(data.data()[bwritten - 1]),size - bwritten);
                }
                foundSource = true;
                break;
            }
        }

        if (!foundSource) { // or we open a new connection

            QLocalSocket * socket = new(std::nothrow) QLocalSocket();
            if (socket == nullptr) {
                qFatal("Cannot allocate memory for QLocalSocket X{");
            }
            connect(socket, &QLocalSocket::readyRead, this, &PipeClientListener::dataReceived);
            //connect(socket, qOverload<QLocalSocket::LocalSocketError>(&QLocalSocket::error),this, &PipeClientListener::onSocketError,Qt::QueuedConnection);
            connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onSocketError(QLocalSocket::LocalSocketError)), Qt::QueuedConnection);
            connect(socket, &QLocalSocket::disconnected, this, &PipeClientListener::onClientDeconnection,Qt::QueuedConnection);

            socket->connectToServer(pipeName, QIODevice::ReadWrite);
            if (socket->isOpen()) {
                sid = BlocksSource::newSourceID(this);
                PipeConnection cd;
                cd.sid = sid;
                cd.pipeName = pipeName;
                cd.creationTimeStamp = QDateTime::currentDateTime();

                sockets.insert(socket,cd);

                if (sockets.size() > 10000)
                    emit log(tr("The number of Pipe clients connections as reached 10 000. Dude for real ?"),ID, Pip3lineConst::LERROR);
                mapExtSourcesToLocal.insert(bid, sid);
                emit log(tr("Opening a new connection: %1").arg(sid),ID,Pip3lineConst::LSTATUS);

                qint64 bwritten = socket->write(data);
                while (size > bwritten) {
                    bwritten += socket->write(&(data.data()[bwritten - 1]),size - bwritten);
                }

                updateConnectionsInfo();
            }
        }
    } else {
        emit log(tr("The client is not started, cannot send the packet"),ID,Pip3lineConst::LERROR);
    }
}

bool PipeClientListener::startListening()
{
    qDebug() << tr("[PipeClientListener::startListening]");
    if (!running) {
        // really basic
        running = true;
        emit started();
        updateConnectionsInfo();
    }

    return true;
}

void PipeClientListener::stopListening()
{
    if (running) {
        running = false;
        qDebug() << tr("[PipeClientListener::stopListening]");
        QHashIterator<QLocalSocket *, PipeConnection> i(sockets);
        while (i.hasNext()) {
            i.next();
            BlocksSource::releaseID(i.value().sid);
            delete i.key();
        }

        sockets.clear();
        mapExtSourcesToLocal.clear();

        emit stopped();
        triggerUpdate();
    }
}

void PipeClientListener::setPipeName(const QString &name)
{
    pipeName = name;
}

void PipeClientListener::createConnection()
{
    QLocalSocket * socket = new(std::nothrow) QLocalSocket();
    if (socket == nullptr) {
        qFatal("Cannot allocate memory for QLocalSocket X{");
    }
    connect(socket, &QLocalSocket::readyRead, this, &PipeClientListener::dataReceived);
    //connect(socket, qOverload<QLocalSocket::LocalSocketError>(&QLocalSocket::error),this, &PipeClientListener::onSocketError,Qt::QueuedConnection);
    connect(socket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onSocketError(QLocalSocket::LocalSocketError)), Qt::QueuedConnection);
    connect(socket, &QLocalSocket::disconnected, this, &PipeClientListener::onClientDeconnection,Qt::QueuedConnection);

    socket->connectToServer(pipeName, QIODevice::ReadWrite);
    if (socket->isOpen()) {
        sid = BlocksSource::newSourceID(this);
        PipeConnection cd;
        cd.sid = sid;
        cd.pipeName = pipeName;
        cd.creationTimeStamp = QDateTime::currentDateTime();

        sockets.insert(socket,cd);

        if (sockets.size() > 10000)
            emit log(tr("The number of Pipe clients connections as reached 10 000. Dude for real ?"),ID, Pip3lineConst::LERROR);

        emit log(tr("Opening a new connection: %1").arg(sid),ID,Pip3lineConst::LSTATUS);

        updateConnectionsInfo();
    }
}

void PipeClientListener::dataReceived()
{
    QLocalSocket * socket = dynamic_cast<QLocalSocket *>(sender());
    if (socket != nullptr) {
        if (!sockets.contains(socket)) {
            qCritical() << tr("[PipeClientListener::dataReceived] Unknown client T_T");
            return;
        }
        int sid = sockets.value(socket).sid;

        if (socket->bytesAvailable() > 0) {
            QByteArray data;

            data = socket->readAll();

            if (data.isEmpty()){
                qCritical() << tr("[PipeClientListener::dataReceived] Received data block is empty, ignoring. T_T");
                return;
            }

            QHashIterator<int, int> i(mapExtSourcesToLocal);
            while (i.hasNext()) {
                i.next();
                if (i.value() == sid)
                    sid = i.key();
            }

            Block * datab = new(std::nothrow) Block(data,sid);
            if (datab == nullptr) qFatal("[PipeClientListener::dataReceived] Cannot allocate Block for PipeClientListener X{");

            emit blockReceived(datab);
        }
    } else {
        qCritical() << tr("[PipeClientListener::dataReceived] cast failed T_T");
    }
}

void PipeClientListener::onSocketError(QLocalSocket::LocalSocketError socketError)
{
    qDebug() << "[PipeClientListener::onError]" << socketError;
    QLocalSocket * sobject = dynamic_cast<QLocalSocket *>(sender());
    if (sobject != nullptr) {
        emit log(sobject->errorString(), actualID, Pip3lineConst::LERROR);
    } else {
        qDebug() << "[PipeClientListener::onError] a connection was closed";
    }
}

void PipeClientListener::onClientDeconnection()
{
    QObject *obj = sender();
    if (obj != nullptr) {
        QLocalSocket * socket = dynamic_cast<QLocalSocket *>(obj);
        if (socket != nullptr) {
            emit log(tr("Disconnection for %1").arg(socket->serverName()), ID, Pip3lineConst::LSTATUS);
            if (sockets.contains(socket)) {
                int cid = sockets.take(socket).sid;

                int mid = Block::INVALID_ID;
                QHashIterator<int, int> i(mapExtSourcesToLocal);
                while (i.hasNext()) {
                    i.next();
                    if (i.value() == cid) {
                        mid = i.key();
                        emit connectionClosed(mid);
                        break;
                    }
                }

                // cleaning any mappings
                if (mid != Block::INVALID_ID)
                    mapExtSourcesToLocal.remove(mid);

                BlocksSource::releaseID(cid);
                delete socket;
                updateConnectionsInfo();
            }
        } else {
            qCritical() << "[PipeClientListener::onClientDeconnection] sender is casted to nullptr";
        }
    } else {
        qDebug() << "[PipeClientListener::onClientDeconnection] sender is nullptr, usually because the connection was closed on our end";
    }
}

QWidget *PipeClientListener::requestGui(QWidget *parent)
{
    PipeClientWidget *pgui = new(std::nothrow) PipeClientWidget(parent);
    connect(pgui, &PipeClientWidget::newPipeName, this, &PipeClientListener::setPipeName);
    return pgui;
}

void PipeClientListener::internalUpdateConnectionsInfo()
{
    connectionsInfo.clear();
    if (running) { // accepting new connections
        Target<BlocksSource *> tac;
        tac.setConnectionID(Block::INVALID_ID);
        QString desc;
        desc = desc.append(QString("[%1] %2")
                .arg(BlocksSource::NEW_CONNECTION_STRING)
                .arg(pipeName));
        tac.setDescription(desc);
        tac.setSource(this);
        connectionsInfo.append(tac);

        QHashIterator<QLocalSocket *, PipeConnection> i(sockets);
        while (i.hasNext()) {
            i.next();
            QString desc;
            PipeConnection pc = i.value();
            desc = desc.append(QString("[%1:%2] %3")
                    .arg(pc.sid)
                    .arg(pc.creationTimeStamp.toString())
                    .arg(pipeName));

            Target<BlocksSource *> tac;
            tac.setDescription(desc);
            tac.setConnectionID(pc.sid);
            tac.setSource(this);
            connectionsInfo.append(tac);
        }
    }
}
