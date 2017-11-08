/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "basicsource.h"
#include <QDebug>
#include <QBuffer>
#include <QTime>
#include <QFile>
#include <QFileInfo>
#include "state/basestateabstract.h"
#include "shared/guiconst.h"

BasicSourceReader::BasicSourceReader(QByteArray *source) :
    buffer(source,0)
{
    if (!buffer.open(QIODevice::ReadOnly)) {
        qCritical() << tr("Could not open buffer for reading T_T");
    }
    rawSource = nullptr;
    size = 0;
    offset = 0;
}

BasicSourceReader::~BasicSourceReader()
{
    buffer.close();
}

bool BasicSourceReader::seek(quint64 pos)
{
    if (pos > INT_MAX) {
        emit log(tr("pos is too large for seeking, ignoring"),metaObject()->className(),Pip3lineConst::LERROR);
        return false;
    }
    return buffer.seek((qint64)pos);
}

int BasicSourceReader::read(char *cbuf, int maxLen)
{
    if (maxLen > MAX_READ_SIZE) { // forcing small read
        emit log(tr("maxLen is too large for reading, reducing it."),metaObject()->className(),Pip3lineConst::LWARNING);
        maxLen = MAX_READ_SIZE; // MAX_READ_SIZE is small
    }
    return (int)buffer.read(cbuf,(qint64)maxLen); // maxLen is small, so no issue in conversion
}

bool BasicSourceReader::isReadable()
{
    return buffer.isOpen() && buffer.isReadable();
}

const int BasicSearch::SearchBlockSize = 0x3000000; // ~ 50Mb
//const int BasicSearch::SearchBlockSize = 0x7000000; // ~ 115Mb

BasicSearch::BasicSearch(QByteArray *data)
{
    sdata = data;
}

BasicSearch::~BasicSearch()
{

}

void BasicSearch::internalStart()
{
    if (sdata == nullptr) {
         emit log(tr("Null search data"), this->metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }

    totalSearchSize = sdata->size();
    qint64 dataSize = sdata->size();
    qint64 itemSize = sitem.size();

    if (itemSize > dataSize) {
        emit errorStatus(true);
        return;
    }

    int numBlock = dataSize / SearchBlockSize + ((sdata->size() % SearchBlockSize) == 0 ? 0 : 1);

    int i = 0;
    for (i = 0; i < numBlock; i++) {
        SearchWorker * sw = nullptr;
        BasicSourceReader * buffer = new(std::nothrow)BasicSourceReader(sdata);
        if (buffer == nullptr) {
            qFatal("Cannot allocate memory for QBuffer X{");
        }

        connect(buffer, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)));

        sw = new(std::nothrow) SearchWorker(buffer);
        if (sw == nullptr) {
            qFatal("Cannot allocate memory for SearchWorker X{");
        }
        sw->setStartOffset(i * SearchBlockSize);
        sw->setEndOffset(qMin((quint64)(i * SearchBlockSize + SearchBlockSize),totalSearchSize) - 1); // -1 due to size values in use
        sw->setSearchItem(sitem,mask);
        addSearchWorker(sw);
    }
}

const QByteArray BasicSource::TEXT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ12345678 90<>,./?#'@;:!$~%^&*()_-+=\\|{}`[]");
const QString BasicSource::LOGID("BasicSource");

BasicSource::BasicSource(QObject *parent) :
    ByteSourceAbstract(parent)
{
    bsearchObj = nullptr;
    _name = tr("Basic source");
    capabilities = CAP_RESET | CAP_RESIZE | CAP_HISTORY | CAP_TRANSFORM | CAP_LOADFILE | CAP_SEARCH | CAP_WRITE | CAP_COMPARE;
   // qDebug() << "Created: " << this;
}

BasicSource::~BasicSource()
{
   // qDebug() << "Destroyed: " << this;
}

QString BasicSource::description()
{
    return name();
}

void BasicSource::setData(QByteArray data, quintptr source)
{
    if (!checkReadOnly()) {
        historyAddReplace(0,rawData,data);
        setRawData(data, source);
    }

}

QByteArray BasicSource::getRawData()
{
    return rawData;
}

void BasicSource::setRawData(QByteArray data, quintptr source)
{
    rawData = data;
    emit updated(source);
    emit sizeChanged();
    emit reset();
}

quint64 BasicSource::size()
{
    return rawData.size();
}

QByteArray BasicSource::extract(quint64 offset, int length)
{
    if (!validateOffsetAndSize(offset, 0))
        return QByteArray();

    if (length < 0) {
        offset = (offset + length + 1);
        length = qAbs(length);
    }

    return rawData.mid(offset,length);
}

char BasicSource::extract(quint64 offset)
{
    if (!validateOffsetAndSize(offset, 1)) {
        return '\00';
    }
    return rawData.at(offset);
}

void BasicSource::replace(quint64 offset, int length, QByteArray repData, quintptr source)
{
    if (!_readonly && validateOffsetAndSize(offset, length)) {
        historyAddReplace(offset, rawData.mid(offset,length),repData);
        rawData.replace(offset,length,repData);

        if (!staticMarking) {
            int diff = length - repData.size(); // we (safely) assume that size() cannot be negative
            BytesRange::moveMarkingAfterReplace(userMarkingsRanges, offset, diff);
        }
        emit updated(source);
        if (length != repData.size())
            emit sizeChanged();
    }
}

void BasicSource::insert(quint64 offset, QByteArray repData, quintptr source)
{
    if (!_readonly && validateOffsetAndSize(offset, 0)) {
        historyAddInsert(offset,repData);
        rawData.insert(offset, repData);
        if (!staticMarking) BytesRange::moveMarkingAfterInsert(userMarkingsRanges, offset,repData.size());
        emit updated(source);
        emit sizeChanged();
    }
}

void BasicSource::remove(quint64 offset, int length, quintptr source)
{
    if (!_readonly && validateOffsetAndSize(offset, 0)) {
        historyAddRemove(offset,rawData.mid(offset,length));
        rawData.remove(offset, length);

        if (!staticMarking) {
            quint64 end = offset;
            if (length < 0) {
                length = qAbs(length);
                if (offset < (quint64)length) {
                    offset = 0;
                } else {
                    offset = offset - length;
                }
            } else {
                if (offset + (quint64)length > (quint64)rawData.size())
                    end = rawData.size();
                else
                    end = offset + (quint64)length - 1;
            }

            BytesRange::clearMarkingFromList(userMarkingsRanges, offset,end);
            BytesRange::moveMarkingAfterDelete(userMarkingsRanges, offset,length);
        }
        emit updated(source);
        emit sizeChanged();
    }
}

void BasicSource::clear(quintptr source)
{
    if (!_readonly) {
        historyAddRemove(0,rawData);
        rawData.clear();
        emit updated(source);
        emit sizeChanged();
    }
}

int BasicSource::getViewOffset(quint64 realoffset)
{
    if (realoffset > (quint64) rawData.size()) {
        emit log(tr("Offset too large: %1").arg(realoffset),LOGID, Pip3lineConst::LERROR);
        return - 1;
    }
    return (int)realoffset;
}

int BasicSource::preferredTabType()
{
    return TAB_TRANSFORM;
}

bool BasicSource::isOffsetValid(quint64 offset)
{
    return offset < ((quint64)rawData.size());
}

bool BasicSource::isReadableText()
{
    if (rawData.contains('\x00'))
        return false;
    int count = 0;

    for (int i=0; i < rawData.size(); i++) {
        if (TEXT.contains(rawData.at(i)))
            count++;
    }
    if ((float)(count)/rawData.size() < 0.7) {
        return false;
    }
    return true;
}

void BasicSource::fromLocalFile(QString fileName)
{
    QFileInfo finfo(fileName);
    if (!finfo.isReadable()) {
        emit log(tr("File %1 is not readable with the current process permissions, ignoring load request.").arg(fileName),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    if (finfo.size() > INT_MAX) {
        emit log(tr("Cannot load %1 in memory, as its size is above the hard limit (INT_MAX). Use the Large File view instead").arg(fileName),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }

    clearAllMarkings();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        emit log(tr("Failed to open %1:\n %2").arg(fileName).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }

    setData(file.readAll());
    file.close();
    setName(finfo.fileName());
}

BaseStateAbstract *BasicSource::getStateMngtObj()
{
    BasicSourceStateObj *stateObj = new(std::nothrow) BasicSourceStateObj(this);
    if (stateObj == nullptr) {
        qFatal("Cannot allocate memory for BasicSourceStateObj X{");
    }

    return stateObj;
}

SearchAbstract *BasicSource::requestSearchObject(QObject *)
{
    BasicSearch *sobj = new(std::nothrow) BasicSearch(&rawData);
    if (sobj == nullptr) {
        qFatal("Cannot allocate memory for BasicSearch X{");
    }
    connect(sobj, SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), SIGNAL(log(QString,QString,Pip3lineConst::LOGLEVEL)), Qt::QueuedConnection);
    connect(sobj, SIGNAL(foundList(BytesRangeList*)), SLOT(setNewMarkings(BytesRangeList*)), Qt::QueuedConnection);
    return sobj;
}

bool BasicSource::validateOffsetAndSize(quint64 offset, int length)
{
    if (offset > (quint64)rawData.size()) { // hitting the limit data size
        emit log(tr("Offset too large: %1 length: %2").arg(offset).arg(length),LOGID, Pip3lineConst::LERROR);
        return false;
    }

    if (length < 0 ) { // trivial
        emit log(tr("Negative length: %2").arg(length),LOGID, Pip3lineConst::LERROR);
        return false;
    }

    if ((quint64)(INT_MAX - length) < offset) { // // hitting the limit
        emit log(tr("Length too large, hitting the int MAX limit. offset: %1 length: %2").arg(offset).arg(length),LOGID,Pip3lineConst::LWARNING);
        return false;
    }

    if (offset + (quint64)length > (quint64)rawData.size()) { // this is behond the end of the data
        emit log(tr("Length too large for the data set. offset: %1 length: %2").arg(offset).arg(length),LOGID,Pip3lineConst::LWARNING);
        return false;
    }

    return true;
}


BasicSourceStateObj::BasicSourceStateObj(BasicSource *bs) :
    ByteSourceStateObj(bs)
{
    setName(bs->metaObject()->className());
}

BasicSourceStateObj::~BasicSourceStateObj()
{

}

void BasicSourceStateObj::internalRun()
{
    QByteArray rawData;
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        if (flags & GuiConst::STATE_LOADSAVE_DATA) {
            rawData = static_cast<BasicSource *>(bs)->getRawData();
            QTime timer;
            timer.start();
            writer->writeTextElement(GuiConst::STATE_DATA, write(rawData));
        }
    }else {
        bs->clear();

        if (readNextStart(GuiConst::STATE_DATA)) {
            rawData = readByteArray(reader->readElementText());
            static_cast<BasicSource *>(bs)->setRawData(rawData);
        }
    }
}
