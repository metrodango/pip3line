/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "currentmemorysource.h"
#include <climits>
#include <QWidget>
#include <QDebug>
#include <QFile>
#include "memorywidget.h"
#include <QSysInfo>
#ifdef Q_OS_LINUX
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#elif defined(Q_OS_WIN)
#include "Windows.h"
#elif defined(Q_OS_OSX)
#include <sys/errno.h>
#include <sys/mman.h>
#include <unistd.h>
#endif


MemSearch::MemSearch(CurrentMemorysource *source)
{

    ranges = source->getMemRangesModel()->getList();
}

MemSearch::~MemSearch()
{

}

void MemSearch::internalStart()
{
//    int i = 0;
//    int totalSearchBlocks = ranges.size();
//    for (i = 0; i < totalSearchBlocks; i++) {

//        worker = new(std::nothrow) SearchWorker(file);

//        if (worker == nullptr) {
//            qFatal("Cannot allocate memory for SearchWorker X{");
//        } else {
//            worker->setStartOffset(i * blocksize);
//            worker->setEndOffset(qMin((i+1) * blocksize + sitem.size(),fsize)); // qMin is for the last block
//            worker->setStatsStep((quint64)((double)totalSearchSize * 0.01)); // setting stats steps at 1% of the total size
//            addSearchWorker(worker);
//        }
//    }
}


MemRange::MemRange(quint64 lowerVal, quint64 upperVal, QString description):
    BytesRange(lowerVal, upperVal, description)
{
    read = false;
    write = false;
    exec = false;
#if defined(Q_OS_WIN)
    copy = false;
#endif
    priv = false;
}

void MemRange::setExec(bool val)
{
    exec = val;
}

bool MemRange::isExec() const
{
    return exec;
}

void MemRange::setRead(bool val)
{
    read = val;
}

bool MemRange::isRead() const
{
    return read;
}

void MemRange::setWrite(bool val)
{
    write = val;
}

bool MemRange::isWrite() const
{
    return write;
}

void MemRange::setPriv(bool val)
{
    priv = val;
}

bool MemRange::isPriv() const
{
    return priv;
}

QString MemRange::toString()
{
    QString ret;
    ret.append(QString::number(lowerVal,16)).append("->");
    ret.append(QString::number(upperVal,16)).append(" ");
    ret.append(read ? "r" : "_");
    ret.append(write ? "w" : "_");
    ret.append(exec ? "x" : "_");
#if defined(Q_OS_WIN)
    ret.append(copy ? "c" : "_");
#endif
    ret.append(priv ? "p" : "_");
    ret.append(":").append(description);

    return ret;

}

bool MemRange::operator<(const MemRange &other) const
{
    return upperVal < other.lowerVal;
}

#if defined(Q_OS_WIN)
bool MemRange::getCopy() const
{
    return copy;
}

void MemRange::setCopy(bool value)
{
    copy = value;
}
#endif

bool MemRange::lessThanFunc(QSharedPointer<MemRange> or1, QSharedPointer<MemRange> or2)
{ // "overall" less than
    MemRange * br1 = or1.data();
    MemRange * br2 = or2.data();
    if (br1->lowerVal == br2->lowerVal) { // in case both begins the same
        return br1->upperVal < br2->upperVal; // we compare the upper limit
    }
    return br1->lowerVal < br2->lowerVal;
}


const QFont MemRangeModel::RegularFont = QFont("Courier New",10,2);
const QStringList MemRangeModel::headers = QStringList() << "Start" << "End" << "Permissions" << "Size" << "Description" ;

MemRangeModel::MemRangeModel(QObject *parent) :
    QAbstractListModel(parent)
{
    currentMemRow = -1;
}

MemRangeModel::~MemRangeModel()
{
    clear();
}

QList<QSharedPointer<MemRange> > MemRangeModel::getList()
{
    return ranges;
}

bool MemRangeModel::isOffsetInRange(quint64 offset)
{
    bool valid = false;
    for (int i = 0; i < ranges.size(); i++) {
        QSharedPointer<MemRange> r = ranges.at(i);
        if (r->isInRange(offset)) {
            valid = true;
            break;
        }
    }
    return valid ;
}
QSharedPointer<MemRange> MemRangeModel::getRange(const QModelIndex &index)
{
    QSharedPointer<MemRange> ret;
    int i = index.row();
    if (i < ranges.size()) {
        ret = ranges.at(i);
    }
    return ret;
}

QSharedPointer<MemRange> MemRangeModel::getRange(quint64 offset)
{
    QSharedPointer<MemRange> ret;
    for (int i = 0; i < ranges.size(); i++) {
        if (ranges.at(i)->isInRange(offset)) {
            ret = ranges.at(i);
            break;
        }
    }
    return ret;
}

void MemRangeModel::setCurrentMem(const QModelIndex &index)
{
    if (index.isValid())
        currentMemRow = index.row();
}

int MemRangeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return ranges.size();
}

int MemRangeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return 5;
}

QVariant MemRangeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole) {
        int i = index.row();
        if (i < ranges.size()) {
            switch (index.column()) {
                case START_OFFSET:
                return BytesRange::offsetToString(ranges.at(i)->getLowerVal());
                case END_OFFSET:
                    return BytesRange::offsetToString(ranges.at(i)->getUpperVal());
                case PERMISSIONS:
                return QString("%1%2%3%4")
                          .arg(ranges.at(i)->isRead()?"r":"-")
                          .arg(ranges.at(i)->isWrite()?"w":"-")
                          .arg(ranges.at(i)->isExec()?"x":"-")
                          .arg(ranges.at(i)->isPriv()?"p":"-");
                case SIZE:
                        return QString::number(ranges.at(i)->getSize());
                case DESCRIPTION:
                    return ranges.at(i)->getDescription();
            }
        }
    } else if (role == Qt::BackgroundRole && index.row() == currentMemRow) {
        return QVariant(QColor(236,242,118));
    } else if (role == Qt::TextAlignmentRole && index.column() < headers.size()) {
        if (index.column() < PERMISSIONS)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        else if (index.column() == SIZE)
            return QVariant(Qt::AlignRight | Qt::AlignVCenter);
        else if (index.column() == DESCRIPTION)
            return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        else
            return Qt::AlignCenter;
    } else if (role == Qt::FontRole) {
        return RegularFont;
    }
    return QVariant();
}

QVariant MemRangeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        return headers.at(section);
    }
    return QVariant();
}

void MemRangeModel::clear()
{
    beginResetModel();
    while (!ranges.isEmpty()) {
        ranges.takeLast().clear();
    }
    endResetModel();
}

void MemRangeModel::addRange(QSharedPointer<MemRange> range)
{
    if (ranges.contains(range)) {
        return; // nothing to do here
    }
    beginInsertRows(QModelIndex(),ranges.size(),ranges.size());
    ranges.append(range);
    std::sort(ranges.begin(),ranges.end(), MemRange::lessThanFunc);
    endInsertRows();
}

void MemRangeModel::setCurrentRange(QSharedPointer<MemRange> range)
{
    currentMemRow = ranges.indexOf(range);
    emit dataChanged ( createIndex(currentMemRow,0), createIndex(currentMemRow,3));
}

CurrentMemorysource::CurrentMemorysource(QObject *parent) :
    LargeRandomAccessSource(parent)
{
    _readonly = true;
    _name = tr("Current process memory");
    capabilities = CAP_HISTORY;
    currentRange.clear();
    rangesModel = new(std::nothrow) MemRangeModel(this);
    if (rangesModel == nullptr) {
        qFatal("Cannot allocate memory for MemRangeModel X{");
    }
    mapMemory();

    refreshTimer.start(intervalMSec);

    quintptr pwritedata = reinterpret_cast<quintptr>(this);
    setStartingOffset(pwritedata);
//    qDebug() << "writeData" << QString::number((quintptr)&writeData,16);
//    qDebug() << "memcpy" << QString::number((quintptr)&memcpy,16);
}

CurrentMemorysource::~CurrentMemorysource()
{
    delete rangesModel;
}

QString CurrentMemorysource::description()
{
    return name();
}

quint64 CurrentMemorysource::size()
{
    //qDebug() << "Wordsize = " << QSysInfo::WordSize;
    if (QSysInfo::WordSize == 32)
        return ULONG_MAX;

    return ULLONG_MAX;
}

bool CurrentMemorysource::isOffsetValid(quint64 offset)
{
    return rangesModel->isOffsetInRange(offset);
}

MemRangeModel *CurrentMemorysource::getMemRangesModel() const
{
    return rangesModel;
}

QWidget *CurrentMemorysource::requestGui(QWidget *parent, GUI_TYPE type)
{
    QWidget *ret = nullptr;
    if (type == GUI_CONFIG) {
        MemoryWidget * mw = new(std::nothrow) MemoryWidget(this, parent);
        if (mw == nullptr) {
            qFatal("Cannot allocate memory for MemoryWidget X{");
        }
        mw->setProcSelection(false);
        ret = mw;
    }
    return ret;
}

void CurrentMemorysource::mapMemory()
{
    rangesModel->clear();
#ifdef Q_OS_LINUX
    QFile pfile("/proc/self/maps");
    if (!pfile.open(QIODevice::ReadOnly)) {
        emit log(tr("Cannot open /proc/self/maps for reading"),metaObject()->className(), Pip3lineConst::LERROR);
        return;
    }

    QList<QByteArray> mappings = pfile.readAll().split('\n');
    for (int i = 0 ; i < mappings.size(); i++) {
        QList<QByteArray> entries = mappings.at(i).split(' ');
        if (entries.size() > 6) {

            QList<QByteArray> range = entries.at(0).split('-');
            QSharedPointer<MemRange> memrange(new(std::nothrow) MemRange(range.at(0).toULongLong(nullptr,16),
                                                                         range.at(1).toULongLong(nullptr,16) - 1,
                                                                         QString::fromUtf8(entries.last())));
            if (memrange == nullptr) {
                qFatal("Cannot allocate memory for MemRange X{");
            }
            memrange->setRead(entries.at(1).at(0) == 'r');
            memrange->setWrite(entries.at(1).at(1) == 'w');
            memrange->setExec(entries.at(1).at(2) == 'x');
            memrange->setPriv(entries.at(1).at(3) == 'p');
            rangesModel->addRange(memrange);
            //qDebug() << memrange->toString();
        }
    }

    emit mappingChanged();

#elif defined(Q_OS_WIN)
    ULONG_PTR addrCurrent = 0;
    ULONG_PTR lastBase = (-1);
    for(;;)
    {
#ifdef Q_OS_WIN64
        MEMORY_BASIC_INFORMATION64 memMeminfo;
#else
        MEMORY_BASIC_INFORMATION32 memMeminfo;
#endif
        VirtualQuery(reinterpret_cast<LPVOID>(addrCurrent), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&memMeminfo), sizeof(memMeminfo) );

        if(lastBase == (ULONG_PTR) memMeminfo.BaseAddress) {
            break;
        }

        lastBase = (ULONG_PTR) memMeminfo.BaseAddress;

        if(memMeminfo.State == MEM_COMMIT) {
            QSharedPointer<MemRange> memrange = QSharedPointer<MemRange>(new(std::nothrow) MemRange((quint64)memMeminfo.BaseAddress,
                                                            (quint64)(memMeminfo.BaseAddress + memMeminfo.RegionSize - 1)
                                                            ));
            switch (memMeminfo.AllocationProtect)
            {
                case PAGE_EXECUTE:
                    memrange->setExec(true);
                    break;
                case PAGE_EXECUTE_READ:
                    memrange->setRead(true);
                    memrange->setExec(true);
                    break;
                case PAGE_EXECUTE_READWRITE:
                    memrange->setRead(true);
                    memrange->setExec(true);
                    memrange->setWrite(true);
                    break;
                case PAGE_EXECUTE_WRITECOPY:
                    memrange->setExec(true);
                    memrange->setWrite(true);
                    memrange->setCopy(true);
                    break;
                case PAGE_READONLY:
                    memrange->setRead(true);
                    break;
                case PAGE_READWRITE:
                    memrange->setRead(true);
                    memrange->setExec(true);
                    memrange->setWrite(true);
                    break;
                case PAGE_WRITECOPY:
                    memrange->setWrite(true);
                    memrange->setCopy(true);
                    break;
                case PAGE_NOACCESS:
                default: // nothing to set
                    break;
            }

            switch (memMeminfo.Type){
                case MEM_IMAGE:
                    memrange->setDescription(tr("Image"));
                    break;
                case MEM_MAPPED:
                    memrange->setDescription(tr("Mapped area"));
                    break;
                case MEM_PRIVATE:
                    memrange->setDescription(tr("Private"));
                    memrange->setPriv(true);
                    break;
            }
            rangesModel->addRange(memrange);
            qDebug() << memrange->toString();
        }
        addrCurrent += memMeminfo.RegionSize;
    }
#endif
}

quint64 CurrentMemorysource::lowByte()
{
    QSharedPointer<MemRange> range(rangesModel->getRange(currentStartingOffset));
    if (!range.isNull())
        return range->getLowerVal();

    emit log(tr("The current starting offset was not found in any registered memory range T_T"),metaObject()->className(), Pip3lineConst::LERROR);
    return currentStartingOffset;
}

quint64 CurrentMemorysource::highByte()
{
    QSharedPointer<MemRange> range(rangesModel->getRange(currentStartingOffset));
    if (!range.isNull())
        return range->getUpperVal();

    emit log(tr("The current starting offset was not found in any registered memory range T_T"),metaObject()->className(), Pip3lineConst::LERROR);
    return currentStartingOffset;
}

bool CurrentMemorysource::tryMoveUp(int sizeToMove)
{
    return tryMoveView(-sizeToMove);
}

bool CurrentMemorysource::tryMoveDown(int sizeToMove)
{
    return tryMoveView(sizeToMove);
}

bool CurrentMemorysource::tryMoveView(int sizeToMove)
{
    quint64 newOffset = 0;
    quint64 upperBound = currentRange->getUpperVal() + 1;
    if (sizeToMove == 0)
        return false;

    if (sizeToMove < 0) {
        if (currentStartingOffset == 0)
            return false; // already at the beginning, nothing to see here
        if (currentStartingOffset < static_cast<quint64>(-1 * sizeToMove)) { // checking how much we can go up
            newOffset = 0;
        } else {
            newOffset = currentStartingOffset + static_cast<quint64>(sizeToMove);
        }

    } else {
        if (ULLONG_MAX - static_cast<quint64>(sizeToMove) - static_cast<quint64>(chunksize)< currentStartingOffset)
            return false; // checking overflow

        if (currentStartingOffset + static_cast<quint64>(sizeToMove) + static_cast<quint64>(chunksize) > upperBound) {
            return false; // no more data
        }

        newOffset = currentStartingOffset + static_cast<quint64>(sizeToMove);
    }

    if (!isOffsetValid(newOffset))
        return false; // just return if the offset is not accessible

    if (!isOffsetValid(newOffset +  static_cast<quint64>(chunksize) - 1))
        return false; // just return if the offset is not accessible

    int readsize = static_cast<int>(qMin(upperBound - newOffset,static_cast<quint64>(chunksize)));
    QByteArray temp;
    if (!readData(newOffset, temp,readsize)) {
        return false;
    }
    currentStartingOffset = newOffset;
    dataChunk = temp;
    emit updated(INVALID_SOURCE);
    emit sizeChanged();
    return true;
}

void CurrentMemorysource::fromLocalFile(QString )
{
    qCritical() << tr("fromLocalFile should never be called on CurrentMemorysource");
}

bool CurrentMemorysource::setStartingOffset(quint64 offset)
{
    bool found = false;
    QList<QSharedPointer<MemRange> > ranges = rangesModel->getList();
    for (int i = 0; i < ranges.size(); i++) {
        if (ranges.at(i)->isInRange(offset)) {
            found = true;
            if (ranges.at(i) != currentRange) {
                currentRange = ranges.at(i);
                rangesModel->setCurrentRange(currentRange);
            }
            QByteArray temp;
            quint64 newOffset = offset;
            quint64 segupper = currentRange->getUpperVal() + 1;

            if (segupper - newOffset < static_cast<quint64>(chunksize)) { // if the data size between offset and the end is inferior to chunksize
                newOffset = segupper - static_cast<quint64>(chunksize); // then starting offset is put back
            }

            if (newOffset % 16 != 0) { // aligning on a 16 bytes boundary
                newOffset = newOffset - newOffset % 16 + (offset >= newOffset + static_cast<quint64>(chunksize) ? 16 : 0);
            }
            if (readData(newOffset,temp,chunksize)) {
                dataChunk = temp;
                currentStartingOffset = newOffset;
                emit updated(INVALID_SOURCE);
                emit sizeChanged();
                addToHistory(newOffset);
                return true;
            }
        }
    }
    if (!found)
        emit log(tr("The current starting offset was not found in any registered memory range T_T"),metaObject()->className(), Pip3lineConst::LERROR);

    return false;
}

bool CurrentMemorysource::readData(quint64 offset, QByteArray &data, int size)
{
    QList<QSharedPointer<MemRange> > ranges = rangesModel->getList();
    // qDebug() << "read current memory data" << QString::number(offset,16).prepend("0x") << size;
    if (size < 0 || ranges.size() < 1) { // trivial
        return false;
    }
    if (size == 0) { // trivial
        data.clear();
        return true;
    }


    QSharedPointer<MemRange> curRange;
    bool valid = false;
    for (int i = 0; i < ranges.size(); i++) {
        curRange = ranges.at(i);
        if (curRange->isInRange(offset)) {
            if (curRange->isRead()) {
                valid = true;
                if (!curRange->isInRange(offset + static_cast<quint64>(size) - 1)) {
                    size = static_cast<int>(curRange->getUpperVal() - offset);
                }
            } else {
                qDebug() << "not readable" << offset;
                emit log(tr("This memory region is not readable [0x%1-0x%2].").arg(QString::number(curRange->getLowerVal(),16)).arg(QString::number(curRange->getUpperVal(),16)),metaObject()->className(), Pip3lineConst::LWARNING);
                return false;
            }
            break;
        }
    }

    if (!valid) {
        qDebug() << "invalid offset";
        emit log(tr("offset %1 is not in a valid memory blocks").arg(QString::number(offset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

#ifdef Q_OS_UNIX
    // testing if the address is readable, just in case the previous check did not worked
    int pipefd[2];
    int ret = pipe(pipefd);
    if (ret != 0) {
        emit log(tr("Cannot create a pipe for testing the address"),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }

    ssize_t ret2 = write(pipefd[1],reinterpret_cast<void *>(offset),static_cast<size_t>(size));
    if (ret2 < 0 && errno != 0) {
        emit log(tr("Cannot access the address 0x%1 for reading [%1]").arg(errorString(errno)),metaObject()->className(), Pip3lineConst::LERROR);
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    } else if (ret2 < size) {
        emit log(tr("Could only read %1 bytes from 0x%2").arg(ret2).arg(QString::number(offset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        size = static_cast<int>(ret2);
    }

    close(pipefd[0]);
    close(pipefd[1]);
#endif
    data.resize(size);
    void *dest = memcpy(data.data(), reinterpret_cast<void *>(offset), static_cast<size_t>(size));

    if (dest != reinterpret_cast<void *>(data.data())) {
        emit log(tr("Uuuh??"),metaObject()->className(), Pip3lineConst::LERROR);
    }

    //qDebug() << "Data read" << QString::number(offset,16).prepend("0x") << data.size();
    return true;
}

bool CurrentMemorysource::writeData(quint64 offset, int length,  const QByteArray &data, quintptr source)
{
    QList<QSharedPointer<MemRange> > ranges = rangesModel->getList();
    bool valid = false;
    QSharedPointer<MemRange> curRange;
    if (length < 0 || ranges.size() < 1)
        return valid;


    for (int i = 0; i < ranges.size(); i++) {
        curRange = ranges.at(i);
        if (curRange->isInRange(offset)) {
            if (curRange->isWrite()) {
                valid = true;
                if (!curRange->isInRange(offset + static_cast<quint64>(length) - 1)) {
                    length = static_cast<int>(curRange->getUpperVal() - offset);
#ifdef Q_OS_UNIX
                    memcpy(reinterpret_cast<void *>(&offset),reinterpret_cast<const void *>(data.data()), static_cast<quint64>(length));
                    emit updated(source);
#elif defined(Q_OS_WIN)
                    qCritical() << tr("writeData not implemented") << data.size() << source;
#endif
                }
            } else {
                qDebug() << "not writable" << offset;
                emit log(tr("This memory region is not writable [0x%1-0x%2].").arg(QString::number(curRange->getLowerVal(),16)).arg(QString::number(curRange->getUpperVal(),16)),metaObject()->className(), Pip3lineConst::LWARNING);
                return false;
            }
            break;
        }
    }





    if (!valid) {
        emit log(tr("offset %1 is not in a valid memory blocks").arg(QString::number(offset,16)),metaObject()->className(), Pip3lineConst::LERROR);
        return false;
    }
    return true;
}

QString CurrentMemorysource::errorString(int errnoVal)
{
#ifdef Q_OS_LINUX
    switch(errnoVal) {
    case EACCES:
        return QString("EACCES");
    case EINVAL:
        return QString("EINVAL");
    case ENOMEM:
        return QString("ENOMEM");
    case EFAULT:
        return QString("EFAULT");
    default:
        return QString("Unmanaged [%1]").arg(errnoVal);
    }
#else
    return QString("Unmanaged [%1]").arg(errnoVal);
#endif
}
