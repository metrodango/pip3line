/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "largerandomaccesssource.h"
#include <climits>
#include <climits>
#include <QDebug>

const int LargeRandomAccessSource::DEFAULT_CHUNKSIZE = 256;

LargeRandomAccessSource::LargeRandomAccessSource(QObject *parent) :
    ByteSourceAbstract(parent)
{
    currentStartingOffset = 0;
    _readonly = true;
    capabilities = CAP_HISTORY;
    currentPointerHistoryPointer = -1;
    chunksize = DEFAULT_CHUNKSIZE;
    sem.release(10);
    intervalMSec = 2000; // 2 sec of interval for the timer
    connect(&refreshTimer, &QTimer::timeout, this, [=](void) {refreshData();});
}

LargeRandomAccessSource::~LargeRandomAccessSource()
{

}

void LargeRandomAccessSource::setData(QByteArray , quintptr )
{
    emit log(tr("Set Data not allowed for this source"),metaObject()->className(), Pip3lineConst::LERROR);
}

QByteArray LargeRandomAccessSource::getRawData()
{
    emit log(tr("Cannot retrive raw data this source"),metaObject()->className(), Pip3lineConst::LERROR);
    return QByteArray();
}

int LargeRandomAccessSource::viewSize()
{
    return qMin(chunksize,dataChunk.size());
}

QByteArray LargeRandomAccessSource::extract(quint64 offset, int length)
{
    QByteArray temp;
    bool ret = readData(offset,temp,length);
    if (!ret) {
        temp.clear();
    }
    return temp;
}

char LargeRandomAccessSource::extract(quint64 offset)
{
    if (offset >= currentStartingOffset && offset < currentStartingOffset + static_cast<quint64>(dataChunk.size())) {
        return dataChunk.at(static_cast<int>(offset - currentStartingOffset));
    }
    QByteArray temp;
    char c = '\00';
    bool ret = readData(offset,temp,1);
    if (ret && temp.size() > 0) {
        c = temp.at(0);
    } else {
        emit log(tr("[extract(quint64 offset)]No byte returned from readData"),metaObject()->className(), Pip3lineConst::LERROR);
    }

    return c;
}

QByteArray LargeRandomAccessSource::viewExtract(int offset, int length)
{
    if (!validateViewOffsetAndSize(offset, 0))
        return QByteArray();

    if (length < 0) {
        offset = (offset + length + 1);
        length = qAbs(length);
    }

    return dataChunk.mid(offset,length);
}

char LargeRandomAccessSource::viewExtract(int offset)
{
    if (!validateViewOffsetAndSize(offset, 1)) {
        return '\00';
    }
    return dataChunk.at(offset);
}

void LargeRandomAccessSource::replace(quint64 offset, int length, QByteArray repData, quintptr source)
{
    if (!writeData(offset,length,repData,source))
        emit log(tr("Error occurred while trying to write data"),metaObject()->className(), Pip3lineConst::LERROR);
    else
        refreshData(false);
}

void LargeRandomAccessSource::viewReplace(int offset, int length, QByteArray repData, quintptr source)
{
    if (validateViewOffsetAndSize(offset, length)) { // if valid
        replace(currentStartingOffset + static_cast<quint64>(offset), length,repData,source);
    }
}


void LargeRandomAccessSource::insert(quint64 , QByteArray , quintptr )
{
    emit log(tr("Insert Data not allowed for this source"),metaObject()->className(), Pip3lineConst::LERROR);
}

void LargeRandomAccessSource::remove(quint64 , int , quintptr )
{
    emit log(tr("Remove Data not allowed for this source"),metaObject()->className(), Pip3lineConst::LERROR);
}

void LargeRandomAccessSource::clear(quintptr )
{
    emit log(tr("Clear Data not allowed for this source"),metaObject()->className(), Pip3lineConst::LERROR);
}

void LargeRandomAccessSource::fromLocalFile(QString)
{
    emit log(tr("load from local file not allowed for this source"),metaObject()->className(), Pip3lineConst::LERROR);
}

void LargeRandomAccessSource::viewMark(int start, int end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    if (start < 0 || end < 0) {
        return;
    }

    if (ULLONG_MAX - static_cast<quint64>(start) < currentStartingOffset || ULLONG_MAX - static_cast<quint64>(end) < currentStartingOffset) {
        return;
    }

    mark(currentStartingOffset + static_cast<quint64>(start), currentStartingOffset + static_cast<quint64>(end), bgcolor, fgColor, toolTip);
}

void LargeRandomAccessSource::viewClearMarking(int start, int end)
{
    if (start < 0 || end < 0) {
        return;
    }

    if (ULLONG_MAX - static_cast<quint64>(start) < currentStartingOffset || ULLONG_MAX - static_cast<quint64>(end) < currentStartingOffset) {
        return;
    }

    clearMarking(currentStartingOffset + static_cast<quint64>(start), currentStartingOffset + static_cast<quint64>(end));
}

QColor LargeRandomAccessSource::getBgViewColor(int pos)
{
    if (pos < 0 || ULLONG_MAX - static_cast<quint64>(pos) < currentStartingOffset) {
        return QColor();
    }

    return getBgColor(currentStartingOffset + static_cast<quint64>(pos));
}

QColor LargeRandomAccessSource::getFgViewColor(int pos)
{
    if (pos < 0 || ULLONG_MAX - static_cast<quint64>(pos) < currentStartingOffset) {
        return QColor();
    }

    return getFgColor(currentStartingOffset + static_cast<quint64>(pos));
}

QString LargeRandomAccessSource::getViewToolTip(int pos)
{
    if (pos < 0 || ULLONG_MAX - static_cast<quint64>(pos) < currentStartingOffset) {
        return QString();
    }

    return getToolTip(currentStartingOffset + static_cast<quint64>(pos));
}

int LargeRandomAccessSource::getViewOffset(quint64 realoffset)
{
    if (!sem.tryAcquire(1,200)) {
        qCritical() << "[LargeRandomAccessSource]Reach getViewOffset max loop depth, returning 0; ";
        return 0;
    }
    int ret = 0;
    int end = chunksize > 1 ? chunksize - 1 : 0;
    if (isOffsetValid(realoffset)) {
        if (realoffset >= currentStartingOffset && realoffset <= (currentStartingOffset + static_cast<quint64>(dataChunk.size()))) {
            realoffset = realoffset - currentStartingOffset;
            if (realoffset <= INT_MAX) {
                ret = static_cast<int>(realoffset);
            } else {
                emit log(tr("Real offset too big %1 T_T").arg(QString::number(realoffset,16)),metaObject()->className(), Pip3lineConst::LERROR);
                ret = end;
            }
        } else if (realoffset < size()) {
           // qDebug() << "looping?" << currentStartingOffset << realoffset;
            if (setStartingOffset(realoffset)) { // will set a new starting offset

                ret = getViewOffset(realoffset); // dangerous ... but simpler
            }
        }
    } else {
        emit log(tr("Offset out-of-bound: %1").arg(QString::number(realoffset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        ret = -1;
    }
    sem.release(1);
    return ret;
}

quint64 LargeRandomAccessSource::getRealOffset(int viewOffset)
{
    quint64 ret = 0;
    if (viewOffset < 0) {
        emit log(tr("View Offset is negative, returning zero"),metaObject()->className(),Pip3lineConst::LERROR);
    } else {
        if (ULLONG_MAX - static_cast<quint64>(viewOffset) < currentStartingOffset) {
            emit log(tr("View Offset is invalid (too large), returning zero"),metaObject()->className(),Pip3lineConst::LERROR);
        } else {
            ret = static_cast<quint64>(viewOffset) + currentStartingOffset;
        }

    }

   // qDebug() << "real offset" << ret;

    return ret;
}

quint64 LargeRandomAccessSource::startingRealOffset()
{
    return currentStartingOffset;
}

bool LargeRandomAccessSource::isReadableText()
{
    return false;
}

bool LargeRandomAccessSource::hasDiscreetView()
{
    return true;
}

int LargeRandomAccessSource::preferredTabType()
{
    return TAB_LARGERANDOM;
}

bool LargeRandomAccessSource::isRefreshEnabled() const
{
    return refreshTimer.isActive();
}

int LargeRandomAccessSource::refreshInterval() const
{
    return intervalMSec;
}

BaseStateAbstract *LargeRandomAccessSource::getStateMngtObj()
{
    LargeRandomAccessSourceStateObj *largeRandomACStateObj = new(std::nothrow) LargeRandomAccessSourceStateObj(this);
    if (largeRandomACStateObj == nullptr) {
        qFatal("Cannot allocate memory for LargeRandomAccessSourceStateObj X{");
    }

    return largeRandomACStateObj;
}

bool LargeRandomAccessSource::historyForward()
{
    if (currentPointerHistoryPointer < pointerHistory.size() - 1) {
        currentPointerHistoryPointer++;
        currentStartingOffset = pointerHistory.at(currentPointerHistoryPointer);
        readData(dataChunk,chunksize);
        emit updated(INVALID_SOURCE);
        emit sizeChanged();
        return true;
    }
    return false;
}

bool LargeRandomAccessSource::historyBackward()
{
    if (currentPointerHistoryPointer > 0) {
        currentPointerHistoryPointer--;
        currentStartingOffset = pointerHistory.at(currentPointerHistoryPointer);
        readData(dataChunk,chunksize);
        emit updated(INVALID_SOURCE);
        emit sizeChanged();
        return true;
    }
    return false;
}

void LargeRandomAccessSource::setViewSize(int size)
{
    if (size > 0 && chunksize != size) {
        QByteArray temp;
        if (readData(temp,size)) {
            dataChunk = temp;
            chunksize = size;
            emit updated(INVALID_SOURCE);
            emit sizeChanged();
        }
    }
}

void LargeRandomAccessSource::refreshData(bool compare)
{
    QByteArray temp;
    if (readData(temp,chunksize)) {
        bool hasSizeChanged = temp.size() !=  dataChunk.size();
        if (hasSizeChanged ||  !dataChunk.contains(temp)) {
            if (compare) {
                clearAllMarkingsNoUpdate();
                int range = qMin(temp.size(), dataChunk.size());
                int selectionStart = -1;
                int i = 0;
                for (i = 0; i < range; i++) {
                    if (temp.at(i) != dataChunk.at(i)) {
                        if (selectionStart == -1)
                            selectionStart = i;
                    } else if (selectionStart > -1) {
                        markNoUpdate(currentStartingOffset + static_cast<quint64>(selectionStart),currentStartingOffset + static_cast<quint64>(i) - 1, QColor(212,137,255));
                        selectionStart = -1;
                    }
                }

                if (selectionStart > -1) {
                    markNoUpdate(currentStartingOffset + static_cast<quint64>(selectionStart),currentStartingOffset + static_cast<quint64>(i), QColor(212,137,255));
                }
            }

            dataChunk = temp;
            if (hasSizeChanged)
                emit sizeChanged();
        }
        emit minorUpdate(0,static_cast<quint64>(dataChunk.size()));
    }
}

void LargeRandomAccessSource::setRefreshEnable(bool refresh)
{
    if (refresh) {
        refreshTimer.start(intervalMSec);
    } else {
        refreshTimer.stop();
    }
}

void LargeRandomAccessSource::setTimerInterval(int msec)
{
    if (intervalMSec != msec) {
        intervalMSec = msec;
        refreshTimer.setInterval(intervalMSec);
    }
}

bool LargeRandomAccessSource::setStartingOffset(quint64 offset)
{
    QByteArray temp;
    quint64 rsize = size();
    quint64 newOffset = offset;
    if (newOffset >= rsize) {
        emit log(tr("Out-of-bound offset: 0x%1").arg(QString::number(newOffset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        newOffset = rsize == 0 ? 0 : rsize - 1; // out-of-bound
    }
    if (rsize < static_cast<quint64>(chunksize)) // if the total size is inferior to the chunksize
        newOffset = 0;

    if (rsize - newOffset < static_cast<quint64>(chunksize)) { // if the data size between offset and the end is inferior to chunksize
        newOffset = rsize - static_cast<quint64>(chunksize) - 1; // then starting offset is put back
       // qDebug() << "Putting back offset" << QString::number(newOffset,16);

    }

    if (newOffset % 16 != 0) {
        newOffset = newOffset - newOffset % 16 + (offset >= newOffset + static_cast<quint64>(chunksize) ? 16 : 0);
       // qDebug() << "...And aligning it" << QString::number(newOffset,16);
    }

    if (readData(newOffset,temp,chunksize)) {
        dataChunk = temp;
        currentStartingOffset = newOffset;
        emit updated(INVALID_SOURCE);
        emit sizeChanged();
        addToHistory(newOffset);
        return true;
    } else {
        qCritical() << "setStartingOffset: could not read data";
    }
    return false;
}

bool LargeRandomAccessSource::validateViewOffsetAndSize(int offset, int length)
{
    if (offset > chunksize) { // chunksize is garanteed to be positive
        emit log(tr("Offset too large: %1 length: %2").arg(offset).arg(length),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    if (length < 0 ) { // trivial
        emit log(tr("Negative length: %2").arg(length),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    if ((INT_MAX - length) < offset) { //
        emit log(tr("Length too large, hitting the INT_MAX limit. offset: %1 length: %2").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
        return false;
    }

    if (offset + length > chunksize) {
        emit log(tr("Length too large, hitting the chunksize limit. offset: %1 length: %2").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
            return false;
    }

    return true;
}

bool LargeRandomAccessSource::readData(QByteArray &data, int size)
{
    return readData(currentStartingOffset,data,size);
}

bool LargeRandomAccessSource::readData(quint64 , QByteArray &, int )
{
    qCritical() << tr("LargeRandomAccessSource::readData not implemented");
    return false;
}

bool LargeRandomAccessSource::writeData(quint64 , int , const QByteArray &, quintptr)
{
    return false; // by default doing nothing
}

void LargeRandomAccessSource::addToHistory(quint64 offset)
{
    currentPointerHistoryPointer++;
    pointerHistory = pointerHistory.mid(0,currentPointerHistoryPointer);
    pointerHistory.append(offset);
}




LargeRandomAccessSourceStateObj::LargeRandomAccessSourceStateObj(LargeRandomAccessSource *ls) :
    ByteSourceStateObj(ls)
{
    setName(ls->name());
}

LargeRandomAccessSourceStateObj::~LargeRandomAccessSourceStateObj()
{

}

void LargeRandomAccessSourceStateObj::internalRun()
{
    LargeRandomAccessSource *ls = static_cast<LargeRandomAccessSource *>(bs);
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeAttribute(GuiConst::STATE_RANDOM_SOURCE_CURRENT_OFFSET, write(ls->startingRealOffset()));
        writer->writeAttribute(GuiConst::STATE_CHUNK_SIZE, write(ls->viewSize()));
    } else {
        QXmlStreamAttributes attrList = reader->attributes();
        bool ok = false;
        quint64 temp = readUInt64(attrList.value(GuiConst::STATE_RANDOM_SOURCE_CURRENT_OFFSET),&ok);
        if (!ok) {
            temp = 0;
        }

        ls->setStartingOffset(temp);

        int cs = readInt(attrList.value(GuiConst::STATE_CHUNK_SIZE),&ok);
        if (!ok) {
            cs = LargeRandomAccessSource::DEFAULT_CHUNKSIZE;
        }

        ls->setViewSize(cs);
    }
}
