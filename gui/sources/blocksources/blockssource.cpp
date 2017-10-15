/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "blockssource.h"
#include <QWidget>
#include "baseblocksourcewidget.h"
#include <QDebug>
#include "shared/guiconst.h"
#include <commonstrings.h>
#include <transformmgmt.h>
#include <QXmlStreamWriter>

const int Block::INVALID_ID = -1;

Block::Block(QByteArray data, int sourceid):
    data(data),
    sourceid(sourceid)
{

}

Block::~Block()
{
    //qDebug() << "Destroying" << this;
}

QByteArray Block::getData() const
{
    return data;
}

void Block::setData(const QByteArray &value)
{
    data = value;
}

int Block::getSourceid() const
{
    return sourceid;
}

void Block::setSourceid(int value)
{
    sourceid = value;
}


int BlocksSource::currentid = 0;
QMutex BlocksSource::idlock;
QHash<int,BlocksSource *> BlocksSource::idSourceTable;
const int BlocksSource::DEFAULT_B64_BLOCK_MAX_SIZE = 0x8000000;
const QString BlocksSource::NEW_CONNECTION_STRING = QObject::tr("New connection");
const QString BlocksSource::NEXT_CLIENT_STRING = QObject::tr("Next client");

BlocksSource::BlocksSource(QObject *parent) :
    QObject(parent),
    flags(0),
    inboundTranform(nullptr),
    outboundTranform(nullptr),
    gui(nullptr)
{
    updateTimer.setInterval(500);
    updateTimer.setSingleShot(true);
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(triggerUpdate()), Qt::QueuedConnection);

    b64MaxBlockLength = DEFAULT_B64_BLOCK_MAX_SIZE;
    b64BlocksSeparator = '\x0a';
    type = INVALID_TYPE;
    sid = BlocksSource::newSourceID(this);
    connect(this, SIGNAL(blockToBeSend(Block *)), SLOT(sendBlock(Block *)), Qt::QueuedConnection);
    //qDebug() << this << "created";
}

BlocksSource::~BlocksSource()
{
    qDebug() << this << "Destroyed";
    delete gui;
    BlocksSource::releaseID(sid);

    delete inboundTranform;
    delete outboundTranform;
}

QWidget *BlocksSource::getGui(QWidget * parent)
{
    if (gui == nullptr) {
        gui = requestGui(parent);
        if (gui != nullptr) {
            connect(gui, SIGNAL(destroyed()), this, SLOT(onGuiDestroyed()), Qt::UniqueConnection);
        }
    }
    return gui;
}

bool BlocksSource::isReadWrite()
{
    return (flags & READ_ONLY) == 0;
}

QWidget *BlocksSource::requestGui(QWidget * parent)
{
    BaseBlockSourceWidget *controlGui = new(std::nothrow) BaseBlockSourceWidget(this,parent);
    if (controlGui == nullptr) {
        qFatal("Cannot allocate memory for DefaultControlGui X{");
    }

    return controlGui;
}

void BlocksSource::processIncomingB64Block(QByteArray data, int rsid)
{

    if (data.size() > 0) {
        QByteArray b64BlockTempData;
        if (b64BlockTempDataList.contains(rsid)) {
            b64BlockTempData = b64BlockTempDataList.value(rsid);
        }

        int count = data.count(b64BlocksSeparator);

        if (count > 0) {
            QList<QByteArray> dataList = data.split(b64BlocksSeparator);
            // checking if the last block is empty , because split can returns empty blocks
            if (dataList.last().isEmpty()) {
               dataList.takeLast();
               // we can remove the block, as the last is complete
               b64BlockTempDataList.remove(rsid);
            } else {
                // keeping the last one in that case (no ending with sep)
                if (dataList.size() > 0) {
                     b64BlockTempDataList.insert(rsid, dataList.takeLast());
                }
            }

            // and sending the rest of them
            while (!dataList.isEmpty()) {
                // without forgetting the heading data
                b64BlockTempData.append(dataList.takeFirst());
                b64DecodeAndEmit(b64BlockTempData, rsid);
                b64BlockTempData.clear();
            }

        } else {
            // appending the data
            b64BlockTempData.append(data);
            if (b64BlockTempData.size() > b64MaxBlockLength) {
                b64BlockTempData.resize(b64MaxBlockLength);
                emit log(tr("Base64 block received is too large, it will be truncated"),metaObject()->className(), Pip3lineConst::LWARNING);
                b64DecodeAndEmit(b64BlockTempData, rsid);
                b64BlockTempDataList.remove(rsid);
            } else {
                b64BlockTempDataList.insert(rsid, b64BlockTempData);
            }
        }
    }
}

QByteArray BlocksSource::applyInboundTransform(QByteArray data)
{
    if (inboundTranform != nullptr) {
        return inboundTranform->transform(data);
    }

    return data;
}

QByteArray BlocksSource::applyOutboundTransform(QByteArray data)
{
    if (outboundTranform != nullptr) {
        return outboundTranform->transform(data);
    }

    return data;
}

void BlocksSource::updateConnectionsInfo()
{
    if (!initialTime.isValid()) {
        initialTime = QTime::currentTime();
        updateTimer.start();
    } else {
        if (initialTime.msecsTo(QTime::currentTime()) < 1000) {
            updateTimer.start();
        } else {
            triggerUpdate();
        }
    }
}

void BlocksSource::internalUpdateConnectionsInfo()
{

}

TransformAbstract *BlocksSource::getOutboundTranform() const
{
    return outboundTranform;
}

void BlocksSource::setOutboundTranform(TransformAbstract *transform)
{
    if (outboundTranform != nullptr) {
        delete outboundTranform;
        outboundTranform = nullptr;
    }
    outboundTranform = transform;
    if (outboundTranform != nullptr)
        emit outboundTranformModfied();
}

TransformAbstract *BlocksSource::getInboundTranform() const
{
    return inboundTranform;
}

void BlocksSource::setInboundTranform(TransformAbstract *transform)
{

    if (inboundTranform != nullptr) {
        delete inboundTranform;
        inboundTranform = nullptr;
    }
    inboundTranform = transform;
    if (inboundTranform != nullptr)
        emit inboundTransformModfied();
}

int BlocksSource::getB64MaxBlockLength() const
{
    return b64MaxBlockLength;
}

void BlocksSource::setB64MaxBlockLength(int value)
{
    b64MaxBlockLength = value;
}

void BlocksSource::sendBlock(Block *block)
{
    qCritical() << tr("[BlocksSource::sendBlock] Not implemented");
    delete block;
}

bool BlocksSource::startListening()
{
    qCritical() << "Not implemented";
    return false; // default
}

int BlocksSource::getSid() const
{
    return sid;
}

BlocksSource::BSOURCETYPE BlocksSource::getType() const
{
    return type;
}

QList<Target<BlocksSource *> > BlocksSource::getAvailableConnections()
{
    QReadLocker lock(&infoLocker);
    return connectionsInfo;
}

bool BlocksSource::isReflexive() const
{
    return (flags & REFLEXION_ENABLED) != 0;
}

void BlocksSource::setReflexive(bool enabled)
{

    if (enabled)
        flags |= REFLEXION_ENABLED;
    else
        flags &= ~REFLEXION_ENABLED;

    emit reflexionChanged(enabled);
}

bool BlocksSource::isB64Blocks() const
{
    return (flags & B64BLOCKS_ENABLED) != 0;
}

void BlocksSource::setB64Blocks(bool enabled)
{
    if (enabled)
        flags |= B64BLOCKS_ENABLED;
    else
        flags &= ~B64BLOCKS_ENABLED;
}

char BlocksSource::getB64BlocksSeparator() const
{
    return b64BlocksSeparator;
}

void BlocksSource::setB64BlocksSeparator(char value)
{
    b64BlocksSeparator = value;
}

void BlocksSource::onConnectionClosed(int cid)
{
    Q_UNUSED(cid); // does nothing by default
}

void BlocksSource::postBlockForSending(Block *block)
{
    emit blockToBeSend(block);
}

void BlocksSource::restart()
{
    stopListening();
    startListening();
}

bool BlocksSource::isTLSEnable() const
{
    return (flags & TLS_ENABLED) != 0;
}

void BlocksSource::setTlsEnable(bool enabled)
{
    if (enabled)
        flags |= TLS_ENABLED;
    else
        flags &= ~TLS_ENABLED;

    emit sslChanged(enabled);
}

void BlocksSource::onGuiDestroyed()
{
    gui = nullptr;
}

void BlocksSource::triggerUpdate()
{
    infoLocker.lockForRead();
    internalUpdateConnectionsInfo();
    infoLocker.unlock();
    initialTime = QTime();
    updateTimer.stop();
    emit updated();
}

void BlocksSource::b64DecodeAndEmit(QByteArray data, int rsid)
{
    data = QByteArray::fromBase64(data);
    if (data.isEmpty()){
        QString mess = tr("Base64 decoded received block is empty, ignoring.");
        emit log(mess,metaObject()->className(), Pip3lineConst::LERROR);
        return;
    }

    data = applyInboundTransform(data);

    Block * datab = new(std::nothrow) Block(data,rsid);
    if (datab == nullptr) qFatal("Cannot allocate Block for TCPListener X{");

    emit blockReceived(datab);
}

quint64 BlocksSource::getFlags() const
{
    return flags;
}

void BlocksSource::setFlags(const quint64 &value)
{
    flags = value;

    // emitting some updates
    emit sslChanged(isTLSEnable());

    emit reflexionChanged(isReflexive());
}

QHash<QString, QString> BlocksSource::getConfiguration()
{
    QHash<QString, QString> ret;
    QByteArray val(1,b64BlocksSeparator);
    ret.insert(Pip3lineConst::XMLSEPARATOR, QString::fromUtf8(val.toHex()));
    ret.insert(Pip3lineConst::XMLBLOCKSIZE, QString::number(b64MaxBlockLength));
    ret.insert(GuiConst::STATE_FLAGS, QString::number(flags));

    TransformMgmt * tfactory = TransformMgmt::getGlobalInstance();
    if (tfactory != nullptr) {
        QString confText;
        QXmlStreamWriter streamin(&confText);

        if (inboundTranform != nullptr) {
            TransformChain list;
            list.append(inboundTranform);
            tfactory->saveConfToXML(list, &streamin);
            ret.insert(GuiConst::STATE_INBOUNDTRANSFORM,confText);
            confText.clear();
        }

        if (outboundTranform != nullptr) {
            TransformChain list;
            list.append(outboundTranform);
            tfactory->saveConfToXML(list, &streamin);
            ret.insert(GuiConst::STATE_OUTBOUNDTRANSFORM,confText);
            confText.clear();
        }

    }
    return ret;
}

void BlocksSource::setConfiguration(QHash<QString, QString> conf)
{
    bool ok = false;

    if (conf.contains(Pip3lineConst::XMLSEPARATOR)) {
        QByteArray tmp = QByteArray::fromHex(conf.value(Pip3lineConst::XMLSEPARATOR).toUtf8());
        if (tmp.size() > 0) {
            b64BlocksSeparator = tmp.at(0);
        }
    }

    if (conf.contains(Pip3lineConst::XMLBLOCKSIZE)) {
        int val = conf.value(Pip3lineConst::XMLBLOCKSIZE).toInt(&ok);
        if (ok && val > 1 && val <= DEFAULT_B64_BLOCK_MAX_SIZE) {
            b64MaxBlockLength = val;
        }
    }

    if (conf.contains(GuiConst::STATE_FLAGS)) {
        quint64 valf = conf.value(GuiConst::STATE_FLAGS).toULongLong(&ok);
        if (ok) {
            setFlags(valf);
        }
    }

    TransformMgmt * tfactory = TransformMgmt::getGlobalInstance();
    if (tfactory != nullptr) {

        if (conf.contains(GuiConst::STATE_INBOUNDTRANSFORM)) {
            TransformChain list = tfactory->loadConfFromXML(conf.value(GuiConst::STATE_INBOUNDTRANSFORM));
            if (list.size() > 0) {
                setInboundTranform(list.at(0));
            }
        }

        if (conf.contains(GuiConst::STATE_OUTBOUNDTRANSFORM)) {
            TransformChain list = tfactory->loadConfFromXML(conf.value(GuiConst::STATE_OUTBOUNDTRANSFORM));
            if (list.size() > 0) {
                setOutboundTranform(list.at(0));
            }
        }
    }
}

int BlocksSource::newSourceID(BlocksSource *source)
{
    QMutexLocker lock(&idlock);

    if (currentid == INT_MAX) {
        qWarning() << "BlocksSource IDs are wrapping";
        if (idSourceTable.size() == INT_MAX) {
            qFatal("BlocksSource IDs table is full ... really? X{");
        }
        currentid = 0;
    }

    while (idSourceTable.contains(currentid)) {
        currentid++;
    }

    idSourceTable.insert(currentid, source);
    return currentid;
}

void BlocksSource::releaseID(int id)
{
    QMutexLocker lock(&idlock);
    idSourceTable.remove(id);
}

BlocksSource *BlocksSource::getSourceObject(int id)
{
    QMutexLocker lock(&idlock);
    return idSourceTable.value(id,nullptr);
}
