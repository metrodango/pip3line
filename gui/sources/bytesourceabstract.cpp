/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include <QFile>
#include <QWidget>
#include <QDebug>
#include <QMapIterator>
#include <QTimerEvent>
#include <QFileInfo>
#include <QApplication>
#include "bytesourceabstract.h"
#include "state/basestateabstract.h"
#include <climits>

const quintptr ByteSourceAbstract::INVALID_SOURCE = 0;

const QString BytesRange::HEXFORMAT = "0x%1";


// utility class used to store ranges of quint64 and associated data
BytesRange::BytesRange(quint64 nlowerVal, quint64 nupperVal, QString ndescription)
{
    if (nlowerVal < nupperVal) {
        lowerVal = nlowerVal;
        upperVal = nupperVal;
    } else {
        lowerVal = nupperVal;
        upperVal = nlowerVal;
    }
    description = ndescription;
    size = upperVal - lowerVal + 1;
}

BytesRange::BytesRange(const BytesRange &other) :
    QObject(nullptr)
{
    *this = other;
}

BytesRange &BytesRange::operator=(const BytesRange &other)
{
    this->description = other.description;
    this->foregroundColor = other.foregroundColor;
    this->backgroundColor = other.backgroundColor;
    this->lowerVal = other.lowerVal;
    this->upperVal = other.upperVal;
    this->size = other.size;
    return *this;
}

BytesRange::~BytesRange()
{
}

bool BytesRange::isInRange(int value)
{
    if (value < 0)
        return false;

    return isInRange(static_cast<quint64>(value));
}

bool BytesRange::isInRange(quint64 value)
{
    return !(value < lowerVal || value > upperVal);
}

QString BytesRange::getDescription() const
{
    return description;
}

void BytesRange::setDescription(const QString &descr)
{
    description = descr;
}

QColor BytesRange::getForeground() const
{
    return foregroundColor;
}

void BytesRange::setForeground(const QColor &color)
{
    foregroundColor = color;
}

QColor BytesRange::getBackground() const
{
    return backgroundColor;
}

void BytesRange::setBackground(const QColor &color)
{
    backgroundColor = color;
}

quint64 BytesRange::getLowerVal() const
{
    return lowerVal;
}

void BytesRange::setLowerVal(quint64 val)
{
    lowerVal = val;
}

quint64 BytesRange::getUpperVal() const
{
    return upperVal;
}

void BytesRange::setUpperVal(quint64 val)
{
    upperVal = val;
}

bool BytesRange::operator<(const BytesRange &other) const
{
    return upperVal < other.lowerVal;
}

bool BytesRange::hasSameMarkings(const BytesRange &other) const
{
    return (backgroundColor == other.backgroundColor) && (foregroundColor == other.foregroundColor) && (description == other.description);
}

void BytesRange::copyMarkings(const BytesRange &other)
{
    backgroundColor = other.backgroundColor;
    foregroundColor = other.foregroundColor;
    description = other.description;
}

QString BytesRange::offsetToString(quint64 val)
{
    return HEXFORMAT.arg(val,16,16,QChar('0'));
}

bool BytesRange::lessThanFunc(QSharedPointer<BytesRange> or1, QSharedPointer<BytesRange> or2)
{ // "overall" less than
    BytesRange * br1 = or1.data();
    BytesRange * br2 = or2.data();
    if (br1->lowerVal == br2->lowerVal) { // in case both begins the same
        return br1->upperVal < br2->upperVal; // we compare the upper limit
    }
    return br1->lowerVal < br2->lowerVal;
}

quint64 BytesRange::getSize() const
{
    return size;
}

void BytesRange::setSize(const quint64 &value)
{
    size = value;
}

void BytesRange::addMarkToList(BytesRangeList *list, QSharedPointer<BytesRange> newRange)
{
    if (list == nullptr) return; // nothing to do here
    if (!(newRange->backgroundColor.isValid() || newRange->foregroundColor.isValid()) && newRange->description.isEmpty()) {
        // apply clearing algo instead
        BytesRange::clearMarkingFromList(list, newRange->lowerVal,newRange->upperVal);
        newRange.clear();
        return;
    }

    bool sorted = false;
    int listsize = list->size();

    for (int i = 0; i < listsize; i++) {
        QSharedPointer<BytesRange> currange = list->at(i);
        bool sameMarkings = currange->hasSameMarkings(*newRange);
        if (newRange->upperVal < currange->lowerVal) { // new range under current
            if (sameMarkings && newRange->upperVal == currange->lowerVal - 1) { // if both range have the same marking and the new range is right under the current (rare)
                currange->lowerVal = newRange->lowerVal;
                newRange.clear(); // cleaning
                sorted = true;
                break;
            }
            // if the new range is "under" the current range, just add it to the list
            if (!list->contains(newRange))
                list->insert(i, newRange);
            sorted = true;
            break;
        } else if (newRange->lowerVal > currange->upperVal) { // new range strictly above current
            if (sameMarkings && newRange->lowerVal == currange->upperVal + 1) { // same markings and right above (rare)
                currange->upperVal = newRange->upperVal; // extend the current range
                newRange.clear();
                newRange = currange; // we still need to check if the extension overlaps with the next ones
                sorted = true; // but no need to add the range (already there)
            }
            // ranges are disjoints just continue, and check the next range
            continue;
        } else if (newRange->lowerVal <= currange->lowerVal) { // overlapping occured
            if (newRange->upperVal >= currange->upperVal) {
                // if new range overlap entirely the current range
                // just remove the old range and continue
                // the iterator will just continue to the next range (or end)
                list->removeAt(i);
                currange.clear();
                i--; // need to update the cursor to take into account the removal
                listsize--;
                continue;
            } else { // new range overlap partially the current range
                if (sameMarkings) { // in case the markings are the same
                    // just extend the current range
                    currange->lowerVal = newRange->lowerVal;
                    int rem = list->removeAll(newRange);
                    if (rem > 1)
                        qWarning() << QObject::tr("more than one range removed ... smells bad T_T");
                    newRange.clear(); // cleaning
                    sorted = true;
                    break;
                } else {
                    // otherwise we have to "reduce" the current range
                    currange->lowerVal = newRange->upperVal + 1;
                    // and store the new one (only if not there already)
                    if (!list->contains(newRange))
                        list->insert(i, newRange);
                    sorted = true;
                    break;
                }
            }

        } else {
            // at this point we know that start > currange->lowerVal and start <= currange->upperVal
            if (sameMarkings) {
            // if markings are the same, all done, no need for the new range, just grow the current one
                if ( newRange->upperVal > currange->upperVal)
                    currange->upperVal = newRange->upperVal;
                newRange.clear();
                newRange = currange;
                sorted = true; // range already in list, no touching
                // we need to continue in case the current range now expand over other ones
                continue;
            }

            // we can already chop the current range , but make sure to conserve the old value
            quint64 oldupperval = currange->upperVal;
            currange->upperVal = newRange->lowerVal - 1;
            if (newRange->upperVal >= oldupperval) {
                // just continue the loop to check with the next range(s)
                continue;
            } else {
                // final case if the new range is included entirely into the current one
                // we need to create a new one after
                QSharedPointer<BytesRange> newRange2(new(std::nothrow)BytesRange(newRange->upperVal + 1,oldupperval));
                newRange2->copyMarkings(*currange);
                //insert the new range (if not there already)
                if (!list->contains(newRange))
                    list->insert(i + 1, newRange);

                // and the newest created one, from the split
                list->insert(list->indexOf(newRange) + 1, newRange2);
                sorted = true;
                break;
            }

        }
    }

    if (!sorted) { // this means that the new range is above all the other ones or this is the first one
        if (!list->contains(newRange))
            list->append(newRange);
    }
}

void BytesRange::addMarkToList(BytesRangeList *list, quint64 start, quint64 end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    if (list == nullptr) return; // nothing to do here
    // stupidity protection
    if (start > end) {
        quint64 temp = start;
        start = end;
        end = temp;
    }

    QSharedPointer<BytesRange> newRange(new(std::nothrow)BytesRange(start,end));
    if (newRange == nullptr) {
        qFatal("Cannot allocate memory for OffsetsRange X{");
    }
    newRange->setBackground(bgcolor);
    newRange->setForeground(fgColor);
    newRange->setDescription(toolTip);
    BytesRange::addMarkToList(list, newRange);
}

void BytesRange::clearMarkingFromList(BytesRangeList *list, quint64 start, quint64 end)
{
    if (list == nullptr) return; // nothing to do here
    // stupidity protection
    if (start > end) {
        quint64 temp = start;
        start = end;
        end = temp;
    }

    int msize = list->size();
    for (int i = 0; i < msize; i++) {
        QSharedPointer<BytesRange> currRange = list->at(i);
        if (start <= currRange->lowerVal) { //possible overlap
            if (end < currRange->lowerVal) // clearing a non-existing marking, nothing to see here
                break;
            else if (end < currRange->upperVal) { // currRange need to be reduced
                currRange->lowerVal = end  + 1;
                break;// we are done here
            } else {// just clear the entire range
                currRange.clear();
                list->removeAt(i);
                i--; // need to take into account the fact that one object was removed;
                msize--;
                continue; // we still need to check if it overlap the next ranges
            }
        } else if (start > currRange->upperVal) { // the clearing range is above the current range
            // just continue to the next one
            continue;
        } else { // start > currRange->lowerVal && start <= currRange->upperVal
            if (end >= currRange->upperVal) { // overlapping above the upper val
                currRange->upperVal = start - 1; // reducing the range up to start (exclusive)
                continue; // need to check the next range
            } else { // some split need to be performed
                QSharedPointer<BytesRange> newRange2(new(std::nothrow)BytesRange(end + 1,currRange->upperVal));
                if (newRange2 == nullptr) {
                    qFatal("Cannot allocate memory for OffsetsRange X{");
                }
                newRange2->copyMarkings(*currRange);
                currRange->upperVal = start - 1; // reducing the current range up to start (exclusive)
                // Insert the newest created one, from the split
                list->insert(i + 1, newRange2);
                break; //job done
            }
        }
    }
}

void BytesRange::moveMarkingAfterDelete(BytesRangeList *list, quint64 pos, quint64 deleteSize)
{
    if (list == nullptr) return; // nothing to do here
    int msize = list->size();
    for (int i = 0; i < msize; i++) {
        QSharedPointer<BytesRange> currRange = list->at(i);
        if (pos > currRange->upperVal) {
            continue;
        }
        if (pos < currRange->lowerVal){
            if (currRange->lowerVal < deleteSize)
                currRange->lowerVal = 0;
            else
                currRange->lowerVal = currRange->lowerVal - deleteSize;
        }

        if (currRange->upperVal < deleteSize) { // at this point the range is ... out of range, removing it
            currRange.clear();
            list->removeAt(i);
            msize--;
        }
        else
            currRange->upperVal = currRange->upperVal - deleteSize;
    }
}

void BytesRange::moveMarkingAfterInsert(BytesRangeList *list, quint64 pos, quint64 insertSize)
{
    if (list == nullptr) return; // nothing to do here
    int msize = list->size();
    for (int i = 0; i < msize; i++) {
        QSharedPointer<BytesRange> currRange = list->at(i);

        if (pos > currRange->upperVal) {
            continue;
        }
        if (pos <= currRange->lowerVal){
            if (currRange->lowerVal > ULLONG_MAX - insertSize){ // at this point the range is ... out of range, removing it
                currRange.clear();
                list->removeAt(i);
                i--; // need to take into account the fact that one object was removed;
                msize--;
                continue;
            }
            else
                currRange->lowerVal = currRange->lowerVal + insertSize;
        }

        if (currRange->upperVal > ULLONG_MAX - insertSize) {
            currRange->upperVal = ULLONG_MAX;
        }
        else
            currRange->upperVal = currRange->upperVal + insertSize;
    }
}

void BytesRange::moveMarkingAfterReplace(BytesRangeList *list, quint64 pos, int diff)
{
    if (diff == 0 || list == nullptr) return; // nothing to do here
    int msize = list->size();
    for (int i = 0; i < msize; i++) {
        QSharedPointer<BytesRange> currRange = list->at(i);

        if (pos > currRange->upperVal) { // probably need some work here
            continue;
        }

        if (diff < 0) { // expansion
            diff = qAbs(diff);
            if (pos < currRange->lowerVal){
                if (currRange->lowerVal > ULLONG_MAX - static_cast<quint64>(diff)){ // at this point the range is ... out of range, removing it
                    currRange.clear();
                    list->removeAt(i);
                    i--; // need to take into account the fact that one object was removed;
                    msize--;
                    continue;
                }
                else
                    currRange->lowerVal = currRange->lowerVal + static_cast<quint64>(diff);
            }

            if (currRange->upperVal > ULLONG_MAX - static_cast<quint64>(diff)) {
                currRange->upperVal = ULLONG_MAX;
            }
            else
                currRange->upperVal = currRange->upperVal + static_cast<quint64>(diff);
        } else { // reduction
            if (pos < currRange->lowerVal){
                if (currRange->lowerVal < static_cast<quint64>(diff)) // diff is positive
                    currRange->lowerVal = 0;
                else
                    currRange->lowerVal = currRange->lowerVal - static_cast<quint64>(diff);
            }

            if (currRange->upperVal < static_cast<quint64>(diff)) { // at this point the range is ... out of range, removing it
                currRange.clear();
                list->removeAt(i);
                msize--;
            }
            else
                currRange->upperVal = currRange->upperVal - static_cast<quint64>(diff);
        }
    }
}

void BytesRange::compareAndMark(const QByteArray &data1, const QByteArray &data2, BytesRangeList *ranges, const QColor &bgcolor, const QColor &fgColor, const QString &toolTip)
{
    int size = qMin(data1.size(), data2.size());
    if (size > 0) {
        int cstart = 0;
        bool markingBytes = false;
        for (int i = 0; i < size; i++) {
            if (data1.at(i) != data2.at(i)) {
                if (!markingBytes) {
                    cstart = i;
                    markingBytes = true;
                }
            } else if (markingBytes) {
                markingBytes = false;
                // i is garanteed to be > 0
                BytesRange::addMarkToList(ranges,
                                          static_cast<quint64>(cstart),
                                          static_cast<quint64>(i - 1),
                                          bgcolor,
                                          fgColor,
                                          toolTip);

            }
        }
        if (markingBytes) {
            BytesRange::addMarkToList(ranges,
                                      static_cast<quint64>(cstart),
                                      static_cast<quint64>(qMax(data1.size(), data2.size()) - 1),
                                      bgcolor,
                                      fgColor,
                                      toolTip);
        }
    }
}

BytesRangeList::BytesRangeList(QObject * parent) :
    QObject(parent)
{
//    qDebug() << this << "created";
}

BytesRangeList::BytesRangeList(const BytesRangeList &other) :
    QObject(other.parent()),
    QList< QSharedPointer<BytesRange> >(other)
{
    emit updated();
}

BytesRangeList &BytesRangeList::operator =(const BytesRangeList &other)
{
    QList<QSharedPointer<BytesRange> >::operator =(other);
    emit updated();
    return *this;
}

BytesRangeList::~BytesRangeList()
{
    //    qDebug() << this << "destroyed";
}

quint64 BytesRangeList::byteSize()
{
    quint64 res = 0;
    for (int i = 0; i < size(); i++) {
        res += at(i)->getSize();
    }
    return res;
}

void BytesRangeList::unify()
{
    int listSize = size();
    if (listSize > 1) {
        std::sort(this->begin(),this->end(), BytesRange::lessThanFunc);

        QSharedPointer<BytesRange> previous = at(0);
        QSharedPointer<BytesRange> current;
        for (int i = 1; i < listSize;) {
            current = this->at(i);
            if (previous->getLowerVal() == current->getLowerVal() &&
                    previous->getUpperVal() == current->getUpperVal()) {
                this->takeAt(i).clear();
                listSize--;
                qWarning() << "[BytesRangeList::unify] duplicate found T_T" << QString::number(previous->getLowerVal(), 16);
            } else {
                previous = current;
                i++;
            }
        }
    }
}

ByteSourceAbstract::ByteSourceAbstract(QObject *parent) :
    QObject(parent)
{
    capabilities = 0;
    trackChanges = false;
    confGui = nullptr;
    buttonBar = nullptr;
    upperView = nullptr;
    searchObj = nullptr;
    cachedRange.clear();
    userMarkingsRanges = nullptr;
    currentHistoryPointer = -1;
    applyingHistory = false;
    staticMarking = false;
    _readonly = false;
}

ByteSourceAbstract::~ByteSourceAbstract()
{
    delete searchObj;

    cachedRange.clear();
    if (userMarkingsRanges != nullptr) {
        disconnect(userMarkingsRanges, &BytesRangeList::destroyed, this, &ByteSourceAbstract::onMarkingsListDeleted);
        while(!userMarkingsRanges->isEmpty())
            userMarkingsRanges->takeFirst().clear();
        delete userMarkingsRanges;
        userMarkingsRanges = nullptr;
    }
    delete confGui;
}

QString ByteSourceAbstract::name()
{
    return _name;
}

void ByteSourceAbstract::setName(QString newName)
{
    if (_name.compare(newName) != 0) {
        _name = newName;
        emit nameChanged(newName);
    }
}

void ByteSourceAbstract::setData(QByteArray, quintptr )
{
}

QByteArray ByteSourceAbstract::getRawData()
{
    return QByteArray();
}

quint64 ByteSourceAbstract::size()
{
    return 0;
}

QByteArray ByteSourceAbstract::viewExtract(int offset, int length)
{
    if (offset < 0 || length < 0) {
        emit log(tr("[viewReplace] Negative offset/length o:%1 l:%2, ignoring").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
        return QByteArray();
    }

    return extract(static_cast<quint64>(offset),length);
}

char ByteSourceAbstract::viewExtract(int offset)
{
    if (offset < 0) {
        emit log(tr("[viewExtract] Negative offset %1, ignoring").arg(offset),metaObject()->className(),Pip3lineConst::LERROR);
        return '\00';
    }
    return extract(static_cast<quint64>(offset));
}

void ByteSourceAbstract::replace(quint64 , int , QByteArray , quintptr )
{
    emit log(tr("[replace] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::viewReplace(int offset, int length, QByteArray repData, quintptr source)
{
    if (offset < 0 || length < 0) {
        emit log(tr("[viewReplace] Negative offset/length o:%1 l:%2, ignoring").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    replace(static_cast<quint64>(offset),length,repData,source);
}

void ByteSourceAbstract::insert(quint64 , QByteArray , quintptr )
{
    emit log(tr("[insert] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::viewInsert(int offset, QByteArray repData, quintptr source)
{
    if (offset < 0) {
        emit log(tr("[viewInsert] Negative offset %1, ignoring").arg(offset),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    insert(static_cast<quint64>(offset),repData,source);
}

void ByteSourceAbstract::remove(quint64, int , quintptr )
{
    emit log(tr("[remove] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::viewRemove(int offset, int length, quintptr source)
{

    if (offset < 0 || length < 0) {
        emit log(tr("[viewRemove] Negative offset/length o:%1 l:%2, ignoring").arg(offset).arg(length),metaObject()->className(),Pip3lineConst::LERROR);
        return;
    }
    remove(static_cast<quint64>(offset),length,source);
}

void ByteSourceAbstract::clear(quintptr)
{
}

bool ByteSourceAbstract::hasCapability(ByteSourceAbstract::CAPABILITIES cap)
{
    return (capabilities & cap);
}

quint32 ByteSourceAbstract::getCapabilities() const
{
    return capabilities;
}

bool ByteSourceAbstract::setReadOnly(bool readonly)
{
    if (!(capabilities & CAP_WRITE))
            return readonly;

    _readonly = readonly;
    emit readOnlyChanged(_readonly);

    return true;
}

bool ByteSourceAbstract::isReadonly()
{
    return !(capabilities & CAP_WRITE) || _readonly;
}

bool ByteSourceAbstract::checkReadOnly()
{
    if (_readonly) {
        emit log(tr("The source is configured as read-only. Enable Read-Write if possible"),metaObject()->className(),Pip3lineConst::LERROR);
    }

    return _readonly;
}

int ByteSourceAbstract::preferredTabType()
{
    return TAB_GENERIC;
}

bool ByteSourceAbstract::isReadableText()
{
    return false;
}

SearchAbstract *ByteSourceAbstract::getSearchObject(QObject * parent, bool singleton)
{
    if (singleton) {
        if (searchObj == nullptr) {
            searchObj = requestSearchObject(parent);
            if (searchObj != nullptr)
                connect(this, &ByteSourceAbstract::reset, searchObj, &SearchAbstract::dataReset);
        }
        return searchObj;
    }
    return requestSearchObject(parent);
}

void ByteSourceAbstract::viewMark(int start, int end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    if (start < 0 || end < 0 ) {
        emit log(tr("[viewMark] Negative end/start %1/%2, ignoring").arg(start).arg(end),metaObject()->className(),Pip3lineConst::LERROR);
    } else {
        mark(static_cast<quint64>(start),static_cast<quint64>(end),bgcolor,fgColor,toolTip);
    }
}

void ByteSourceAbstract::mark(quint64 start, quint64 end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    markNoUpdate(start,end,bgcolor, fgColor, toolTip);
    emit minorUpdate(start,end);
}

void ByteSourceAbstract::markNoUpdate(quint64 start, quint64 end, const QColor &bgcolor, const QColor &fgColor, QString toolTip)
{
    cachedRange.clear();
    if (userMarkingsRanges == nullptr) {
        userMarkingsRanges = new(std::nothrow) BytesRangeList();
        if (userMarkingsRanges == nullptr) {
            qFatal("Cannot allocate memory for BytesRangeList X{");
        }
        connect(userMarkingsRanges, &BytesRangeList::destroyed, this, &ByteSourceAbstract::onMarkingsListDeleted);
    }
    // at this point we know for sure that userMarkingsRanges != nullptr
    BytesRange::addMarkToList(userMarkingsRanges,start,end,bgcolor, fgColor, toolTip);

}

void ByteSourceAbstract::viewClearMarking(int start, int end)
{
    if (start < 0 || end < 0 ) {
        emit log(tr("[viewClearMarking] Negative end/start %1/%2, ignoring").arg(start).arg(end),metaObject()->className(),Pip3lineConst::LERROR);
    } else {
        clearMarking(static_cast<quint64>(start),static_cast<quint64>(end));
    }
}

void ByteSourceAbstract::clearMarking(quint64 start, quint64 end)
{
    cachedRange.clear();
    if (userMarkingsRanges != nullptr) {
        BytesRange::clearMarkingFromList(userMarkingsRanges, start,end);
        emit minorUpdate(start,end);
    }
}

void ByteSourceAbstract::setNewMarkings(BytesRangeList *newUserMarkingsRanges)
{
    if (newUserMarkingsRanges != nullptr) {
        connect(newUserMarkingsRanges, &BytesRangeList::destroyed, this, &ByteSourceAbstract::onMarkingsListDeleted);
        clearAllMarkingsNoUpdate();// implicitely the two lists HAVE to contain different objects pointer, otherwise crash (double free).
        userMarkingsRanges = newUserMarkingsRanges;
        emit minorUpdate(0,size());
    }

}

void ByteSourceAbstract::clearAllMarkings()
{
    clearAllMarkingsNoUpdate();
    emit minorUpdate(0,size());
}

void ByteSourceAbstract::clearAllMarkingsNoUpdate()
{
    cachedRange.clear();
    if (userMarkingsRanges != nullptr) {
        disconnect(userMarkingsRanges, &BytesRangeList::destroyed, this, &ByteSourceAbstract::onMarkingsListDeleted);
        while(!userMarkingsRanges->isEmpty())
            userMarkingsRanges->takeFirst().clear();
        delete userMarkingsRanges;
        userMarkingsRanges = nullptr;
    }
}

bool ByteSourceAbstract::isTrackChanges() const
{
    return trackChanges;
}

void ByteSourceAbstract::setTrackChanges(bool value)
{
    trackChanges = value;
}

BytesRangeList *ByteSourceAbstract::getUserMarkingsRanges() const
{
    return userMarkingsRanges;
}
bool ByteSourceAbstract::hasStaticMarking() const
{
    return staticMarking;
}

void ByteSourceAbstract::setStaticMarking(bool value)
{
    staticMarking = value;
}


void ByteSourceAbstract::onMarkingsListDeleted()
{
    userMarkingsRanges = nullptr;
    cachedRange.clear();
}

bool ByteSourceAbstract::hasMarking() const
{

    return userMarkingsRanges == nullptr ? false : !userMarkingsRanges->isEmpty();
}

QColor ByteSourceAbstract::getBgColor(quint64 pos)
{
    QColor color;
    QSharedPointer<BytesRange> data = cachedRange.toStrongRef();
    if (userMarkingsRanges != nullptr) {
        if (!data.isNull() && data->isInRange(pos)) {
                color = data->backgroundColor;
        } else {
            int size = userMarkingsRanges->size();
            for (int i = 0; i < size ; i++) {
                if (userMarkingsRanges->at(i)->isInRange(pos)) {
                    data = userMarkingsRanges->at(i);
                    cachedRange = data.toWeakRef();
                    color = data->backgroundColor;
                    break;
                }
            }
        }
    }
    return color;
}

QColor ByteSourceAbstract::getBgViewColor(int pos)
{
    if (pos < 0) {
        emit log(tr("[getBgViewColor] Negative offset/length o:%1, ignoring").arg(pos),metaObject()->className(),Pip3lineConst::LERROR);
        return QColor();
    }

    return getBgColor(static_cast<quint64>(pos));
}

QColor ByteSourceAbstract::getFgColor(quint64 pos)
{
    QColor color;
    QSharedPointer<BytesRange> data = cachedRange.toStrongRef();
    if (userMarkingsRanges != nullptr) {
        if (!data.isNull() && data->isInRange(pos)) {
                color = data->foregroundColor;
        } else {
        int size = userMarkingsRanges->size();
            for (int i = 0; i < size ; i++) {
                if (userMarkingsRanges->at(i)->isInRange(pos)) {
                    data = userMarkingsRanges->at(i);
                    cachedRange = data.toWeakRef();
                    color = data->foregroundColor;
                    break;
                }
            }
        }
    }
    data.clear();
    return color;
}

QColor ByteSourceAbstract::getFgViewColor(int pos)
{
    if (pos < 0) {
        emit log(tr("[getFgViewColor] Negative offset/length o:%1, ignoring").arg(pos),metaObject()->className(),Pip3lineConst::LERROR);
        return QColor();
    }
    return getFgColor(static_cast<quint64>(pos));
}

QString ByteSourceAbstract::getToolTip(quint64 pos)
{
    QString texttip;
    QSharedPointer<BytesRange> data  = cachedRange.toStrongRef();
    if (userMarkingsRanges != nullptr) {
        if (!data.isNull() && data->isInRange(pos)) {
                texttip = data->description;
        } else {
            int size = userMarkingsRanges->size();
            for (int i = 0; i < size ; i++) {
                if (userMarkingsRanges->at(i)->isInRange(pos)) {
                    data = userMarkingsRanges->at(i);
                    cachedRange = data.toWeakRef();
                    texttip = data->description;
                    break;
                }
            }
        }
    }

    data.clear();
    return texttip;
}

QString ByteSourceAbstract::getViewToolTip(int pos)
{
    if (pos < 0) {
        emit log(tr("[getViewToolTip] Negative offset/length o:%1, ignoring").arg(pos),metaObject()->className(),Pip3lineConst::LERROR);
        return QString();
    }

    return getToolTip(static_cast<quint64>(pos));
}

QString ByteSourceAbstract::toPrintableString(const QByteArray &val)
{
    QString line;
    for (int i = 0; i < val.size(); i++) {
        line.append((val.at(i) > 32 && val.at(i) < 127) ? val.at(i) : '.');
    }
    return line;
}

quint64 ByteSourceAbstract::lowByte()
{
    return 0;
}

quint64 ByteSourceAbstract::highByte()
{
    return size() - 1;
}

int ByteSourceAbstract::textOffsetSize()
{
    return QString::number(size(),16).size();
}

bool ByteSourceAbstract::tryMoveUp(int )
{
    return false;
}

bool ByteSourceAbstract::tryMoveDown(int )
{
    return false;
}

bool ByteSourceAbstract::hasDiscreetView()
{
    return false;
}

QWidget *ByteSourceAbstract::getGui(QWidget *parent,ByteSourceAbstract::GUI_TYPE type)
{
    QWidget **requestedGui = nullptr;

    switch (type) {
        case (ByteSourceAbstract::GUI_CONFIG):
            requestedGui = &confGui;
            break;
        case (GUI_BUTTONS):
            requestedGui = &buttonBar;
            break;
        case (GUI_UPPER_VIEW):
            requestedGui = &upperView;
            break;
        default:
            qCritical() << tr("Unmanaged ByteSourceAbstract::GUI_TYPE:%1").arg(type);
            return nullptr;
    }

    if ((*requestedGui) == nullptr) {
        (*requestedGui) = requestGui(parent, type);
        if ((*requestedGui) != nullptr) {
            connect((*requestedGui), &QWidget::destroyed, this, &ByteSourceAbstract::onGuiDestroyed, Qt::UniqueConnection);
        }
    }
    return (*requestedGui);
}

int ByteSourceAbstract::viewSize()
{
    quint64 csize = size();
    if (csize > INT_MAX) {
        emit log(tr("Size of the byte source is hitting the INT_MAX limit for the gui view, consider implementing a discreet view"),metaObject()->className(),Pip3lineConst::LERROR);
        csize = INT_MAX;
    }
    return static_cast<int>(csize);
}

void ByteSourceAbstract::setViewSize(int)
{
    emit log(tr("[setViewSize] Not implemented"),metaObject()->className(),Pip3lineConst::LERROR);
}

void ByteSourceAbstract::onGuiDestroyed()
{
    QWidget *gobj = qobject_cast<QWidget *>(sender());
    if (gobj == confGui) {
        confGui = nullptr;
    }
    else if (gobj == buttonBar) {
        buttonBar = nullptr;
    }
    else if (gobj == upperView){
        upperView = nullptr;
    }
    else
        qCritical() << "[ByteSourceAbstract::onGuiDestroyed] Unknown destroyed widget";
}

QWidget *ByteSourceAbstract::requestGui(QWidget *, ByteSourceAbstract::GUI_TYPE)
{
    return nullptr;
}

SearchAbstract *ByteSourceAbstract::requestSearchObject(QObject *)
{
    return nullptr;
}

bool ByteSourceAbstract::historyForward()
{
    if (_readonly) {
        emit log(tr("[historyForward] source is set readonly, ignoring."),metaObject()->className(),Pip3lineConst::LWARNING);
        return false;
    }

    if (currentHistoryPointer < history.size() - 1) {
        currentHistoryPointer++;
        historyApply(history.at(currentHistoryPointer),true);
        return true;
    }
    return false;
}

bool ByteSourceAbstract::historyBackward()
{
    if (_readonly) {
        emit log(tr("[historyBackward] source is set readonly, ignoring."),metaObject()->className(),Pip3lineConst::LWARNING);
        return false;
    }

    if (currentHistoryPointer >= 0) {
        historyApply(history.at(currentHistoryPointer),false);
        currentHistoryPointer--;
        return true;
    }
    return false;
}

void ByteSourceAbstract::historyClear()
{
    currentHistoryPointer = -1;
    history.clear();
}

void ByteSourceAbstract::historyApply(Pip3lineCommon::HistItem item, bool forward)
{
    applyingHistory = true;
    if (item.action == REPLACE) {
        if (forward) {
            replace(item.offset,item.before.length(), item.after);
        } else {
            replace(item.offset,item.after.length(), item.before);
        }
    } else if (item.action == REMOVE) {
        if (forward) {
            remove(item.offset,item.before.length());
        } else {
            insert(item.offset,item.before);
        }
    } else if (item.action == INSERT) {
        if (forward)
            insert(item.offset,item.after);
        else
            remove(item.offset,item.after.length());
    } else {
        emit log(tr("Unknown history action: %1").arg(item.action),metaObject()->className(),Pip3lineConst::LERROR);
    }
    applyingHistory = false;
}

void ByteSourceAbstract::historyAddInsert(quint64 offset, QByteArray after)
{
    if (applyingHistory) // don't touch the history if we are applying a history item
        return;

    HistItem item;
    item.action = INSERT;
    item.offset = offset;
    item.after = after;
    historyAdd(item);
}

void ByteSourceAbstract::historyAddRemove(quint64 offset, QByteArray before)
{
    if (applyingHistory) // don't touch the history if we are applying a history item
        return;

    HistItem item;
    item.action = REMOVE;
    item.offset = offset;
    item.before = before;
    historyAdd(item);
}

void ByteSourceAbstract::historyAddReplace(quint64 offset, QByteArray before, QByteArray after)
{
    if (applyingHistory) // don't touch the history if we are applying a history item
        return;

    HistItem item;
    item.action = REPLACE;
    item.offset = offset;
    item.before = before;
    item.after = after;
    historyAdd(item);
}

void ByteSourceAbstract::historyAdd(Pip3lineCommon::HistItem item)
{
    if (applyingHistory) // don't touch the history if we are applying a history item (this one should not be needed)
        return;

    currentHistoryPointer++;
    history = history.mid(0,currentHistoryPointer);

    history.append(item);
}

void ByteSourceAbstract::writeToFile(QString destFilename, QByteArray data)
{
    // by default write/overwrite the destination file
    if (!destFilename.isEmpty()) {
        QFile file(destFilename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            emit log(tr("Failed to open %1:\n %2").arg(destFilename).arg(file.errorString()),metaObject()->className(),Pip3lineConst::LERROR);
            return;
        }
        qint64 written = 0;
        while ((written = file.write(data)) > 0) {
            if (written == data.length())
                break;
            else
                data = data.mid(static_cast<int>(written) - 1);
        }

        file.close();
    } else {
        emit log(tr("Destination file is empty, ignoring save."),metaObject()->className(),Pip3lineConst::LERROR);
    }
}

void ByteSourceAbstract::saveToFile(QString destFilename, quint64 startOffset, quint64 endOffset)
{
    quint64 t = size();
    if (startOffset < t && endOffset < t) {
        if (endOffset < startOffset) {
            t = startOffset - endOffset + 1;
        } else {
            t = endOffset - startOffset + 1;
        }
        writeToFile(destFilename, extract(startOffset,static_cast<int>(t)));
    } else {
        emit log(tr("Invalid offsets, ignoring save."),metaObject()->className(),Pip3lineConst::LERROR);
    }
}

void ByteSourceAbstract::saveToFile(QString destFilename)
{ // by default we call the getRawData function, but some sources may have more efficient way to do that
  // or/and may not have this function implemented (for instance FileSource)
    writeToFile(destFilename, getRawData());
}

int ByteSourceAbstract::getViewOffset(quint64 realoffset)
{ // return the parameter value casted to int by default
    if (realoffset < INT_MAX) { // checking the int limit
        return static_cast<int>(realoffset);
    } else {
        emit log(tr("Offset value is hitting the INT_MAX limit for the gui view, consider implementing a discreet view"),metaObject()->className(),Pip3lineConst::LERROR);
        return -1;
    }
}

quint64 ByteSourceAbstract::getRealOffset(int viewOffset) // by default return the same value
{
    if (viewOffset < 0) {
        emit log(tr("View Offset is negative, returning zero"),metaObject()->className(),Pip3lineConst::LERROR);
        return 0;
    } else {
        return static_cast<quint64>(viewOffset);
    }
}

quint64 ByteSourceAbstract::startingRealOffset()
{   // return 0 by default as the view size is the same as the data size
    // in case of restricted view, return the real offset of the first item of the view
    return 0;
}

bool ByteSourceAbstract::isOffsetValid(quint64 )
{
    return false;
}


ByteSourceStateObj::ByteSourceStateObj(ByteSourceAbstract *bs) :
    bs(bs)
{

}

ByteSourceStateObj::~ByteSourceStateObj()
{

}

void ByteSourceStateObj::run()
{
    if (flags & GuiConst::STATE_SAVE_REQUEST) {
        writer->writeStartElement(bs->metaObject()->className());
        int size = 0;
        writer->writeAttribute(GuiConst::STATE_NAME, bs->name());
        writer->writeAttribute(GuiConst::STATE_READONLY, write(bs->isReadonly()));
        writer->writeAttribute(GuiConst::STATE_STATIC_MARKINGS, write(bs->hasStaticMarking()));
        writer->writeAttribute(GuiConst::STATE_TRACK_CHANGES, write(bs->isTrackChanges()));

        // inherited class save
        internalRun();

        // remaining data
        BytesRangeList * userMarkingsRanges = bs->userMarkingsRanges;

        if ((flags & GuiConst::STATE_LOADSAVE_DATA) && (flags & GuiConst::STATE_LOADSAVE_MARKINGS) && userMarkingsRanges != nullptr) {
            writer->writeStartElement(GuiConst::STATE_USERMARKINGS);

            size = userMarkingsRanges->size();
            writer->writeAttribute(GuiConst::STATE_SIZE, write(size));
            for (int i = 0; i < size; i++) {
                writer->writeStartElement(GuiConst::STATE_MARKING);
                QSharedPointer<BytesRange> br = userMarkingsRanges->at(i);
                writer->writeAttribute(GuiConst::STATE_BYTE_DESC, write(br->description));
                writer->writeAttribute(GuiConst::STATE_BYTE_LOWER_VAL, write(br->getLowerVal()));
                writer->writeAttribute(GuiConst::STATE_BYTE_UPPER_VAL, write(br->getUpperVal()));
                writer->writeAttribute(GuiConst::STATE_BYTE_FG_COLOR, write(br->getForeground()));
                writer->writeAttribute(GuiConst::STATE_BYTE_BG_COLOR, write(br->getBackground()));
                writer->writeEndElement(); // STATE_MARKING
            }
            writer->writeEndElement(); // STATE_USERMARKINGS
        }

        QList<Pip3lineCommon::HistItem> history = bs->history;
        if ((flags & GuiConst::STATE_LOADSAVE_DATA) && (flags & GuiConst::STATE_LOADSAVE_HISTORY) && !history.isEmpty()) {
            size = history.size();
            writer->writeStartElement(GuiConst::STATE_HEX_HISTORY);
            writer->writeAttribute(GuiConst::STATE_HEX_HISTORY_POINTER, write(bs->currentHistoryPointer));
            writer->writeAttribute(GuiConst::STATE_SIZE, write(size));

            for (int i = 0; i < size; i++) {
                Pip3lineCommon::HistItem hi = history.at(i);
                writer->writeStartElement(GuiConst::STATE_HEX_HISTORY_ITEM);
                writer->writeAttribute(GuiConst::STATE_HEX_HISTORY_OFFSET, write(hi.offset));
                writer->writeAttribute(GuiConst::STATE_HEX_HISTORY_ACTION, write(static_cast<int>(hi.action)));
                writer->writeAttribute(GuiConst::STATE_HEX_HISTORY_BEFORE, write(hi.before));
                writer->writeAttribute(GuiConst::STATE_HEX_HISTORY_AFTER, write(hi.after));
                writer->writeEndElement(); // STATE_HEX_HISTORY_ITEM
            }
            writer->writeEndElement(); // STATE_HEX_HISTORY
        }

        writer->writeEndElement(); // class name
    } else {
        if (readNextStart(bs->metaObject()->className())) {
            ByteSourceClosingObj * closingState = new(std::nothrow) ByteSourceClosingObj(bs);
            if (closingState == nullptr) {
                qFatal("Cannot allocate memory for ByteSourceClosingObj X{");
            }
            emit addNewState(closingState);

            bool ok = false;
            int lsize = 0;
            if (attributes.hasAttribute(GuiConst::STATE_NAME)) {
                QString pname = attributes.value(GuiConst::STATE_NAME).toString();
                if (!pname.isEmpty())
                    setName(pname);
            }

            if (attributes.hasAttribute(GuiConst::STATE_READONLY)) {
                closingState->setReadonly(readBool(attributes.value(GuiConst::STATE_READONLY)));
            }

            if (attributes.hasAttribute(GuiConst::STATE_STATIC_MARKINGS)) {
                bs->setStaticMarking(readBool(attributes.value(GuiConst::STATE_STATIC_MARKINGS)));
            }

            if (attributes.hasAttribute(GuiConst::STATE_TRACK_CHANGES)) {
                bs->setTrackChanges(readBool(attributes.value(GuiConst::STATE_TRACK_CHANGES)));
            }

            // loading inherited class specific
            internalRun();
            // loading the remaining stuff
            if (reader->name() == bs->metaObject()->className()) {
                // if we are still at the start element
                if (reader->tokenType() == QXmlStreamReader::StartElement) {
                    if (reader->readNext() == QXmlStreamReader::Invalid) {
                        qCritical() << "STATE_MARKING loop ended prematurly";
                        return;
                    }
                }
                // the only possibility if this is an endElement
                // is that this is the closing element for the classname, so the
                // confgiguration for this item is empty . We return
                if (reader->tokenType() == QXmlStreamReader::EndElement) {
                    return;
                }
            } else if (reader->tokenType() == QXmlStreamReader::EndElement) { // need to move on here
                if (reader->readNext() == QXmlStreamReader::EndElement) {
                    // if this is also an End element, there should not be anything else for this class
                    return;
                }
            }

            if (reader->name() == GuiConst::STATE_USERMARKINGS) {
                attributes = reader->attributes();

                BytesRangeList * brlist = new(std::nothrow) BytesRangeList();
                if (brlist == nullptr) {
                    qFatal("Cannot allocate memory for BytesRangeList X{");
                }

                if (attributes.hasAttribute(GuiConst::STATE_SIZE)) {
                    lsize = readInt(attributes.value(GuiConst::STATE_SIZE),&ok);
                    if (!ok)
                        lsize = 0;
                    else if (lsize < 1)
                        lsize = 0;
                }

                for (int i = 0; i < lsize; i++) {
                    if (readNextStart(GuiConst::STATE_MARKING)) {

                        quint64 lv = readUInt64(attributes.value(GuiConst::STATE_BYTE_LOWER_VAL),&ok);
                        if (!ok)
                            continue;
                        quint64 uv = readUInt64(attributes.value(GuiConst::STATE_BYTE_UPPER_VAL),&ok);
                        if (!ok)
                            continue;

                        QString desc = readString(attributes.value(GuiConst::STATE_BYTE_DESC));

                        QSharedPointer<BytesRange> bRange(new(std::nothrow) BytesRange(lv,uv,desc));
                        if (bRange == nullptr) {
                            qFatal("Cannot allocate memory for BytesRange X{");
                        }

                        bRange->setBackground(readColor(attributes.value(GuiConst::STATE_BYTE_BG_COLOR)));
                        bRange->setForeground(readColor(attributes.value(GuiConst::STATE_BYTE_FG_COLOR)));

                        brlist->append(bRange);

                        readEndElement(GuiConst::STATE_MARKING);

                    } else { // end loop prematurely
                        qCritical() << "STATE_MARKING loop ended prematurly";
                        break;
                    }

                }
                if (!brlist->isEmpty()) {
                    bs->setNewMarkings(brlist);
               //     qDebug() << "restored " << brlist->size()<< "Markings";
                }
                else {
                    delete brlist;
                }

                readEndAndNext(GuiConst::STATE_USERMARKINGS); // reading closing tag STATE_USERMARKINGS, next tag should be STATE_HEX_HISTORY[StartElement] or classname[EndElement]

            } else {
              //  qDebug() << tr("No user marking saved");
            }

            if (reader->name() == GuiConst::STATE_HEX_HISTORY) {
                attributes = reader->attributes();
                bs->history.clear();

                if (attributes.hasAttribute(GuiConst::STATE_SIZE)) {
                    lsize = readInt(attributes.value(GuiConst::STATE_SIZE),&ok);
                    if (!ok) {
                        qCritical() << "Invalid history size";
                        lsize = 0;
                    }
                } else {
                    qCritical() << reader->name() << "Has no attribute" << GuiConst::STATE_SIZE;
                }
                int index = 0;
                if (attributes.hasAttribute(GuiConst::STATE_HEX_HISTORY_POINTER)) {
                    index = readInt(attributes.value(GuiConst::STATE_HEX_HISTORY_POINTER),&ok);
                    if (!ok)
                        index = 0;
                }

       //         logStatus(QString("History items to restore: %1").arg(lsize));
                for (int i = 0; i < lsize; i++) {
                    if (readNextStart(GuiConst::STATE_HEX_HISTORY_ITEM)) {
                        Pip3lineCommon::HistItem hi;
                        if (attributes.hasAttribute(GuiConst::STATE_HEX_HISTORY_OFFSET)) {
                            hi.offset = readUInt64(attributes.value(GuiConst::STATE_HEX_HISTORY_OFFSET),&ok);
                            if (!ok) {
                                qWarning() << "Error parsing" << GuiConst::STATE_HEX_HISTORY_OFFSET << attributes.value(GuiConst::STATE_HEX_HISTORY_OFFSET).toString();
                                genCloseElement();
                                continue;
                            }
                        } else {
                            qWarning() << "No attribute for STATE_HEX_HISTORY_ITEM, skipping." << GuiConst::STATE_HEX_HISTORY_OFFSET;
                            genCloseElement();
                            continue;
                        }

                        int action = readInt(attributes.value(GuiConst::STATE_HEX_HISTORY_ACTION),&ok);
                        if (!ok || (action != 0 && action != 1 && action != 2)) {
                            qWarning() << "Error parsing STATE_HEX_HISTORY_ACTION, skipping item." << action;
                            continue;
                        }
                        else
                            hi.action = static_cast<Pip3lineCommon::HistAction>(action);

                        hi.before = readByteArray(attributes.value(GuiConst::STATE_HEX_HISTORY_BEFORE));

                        hi.after = readByteArray(attributes.value(GuiConst::STATE_HEX_HISTORY_AFTER));

                        bs->history.append(hi);
                        if (reader->readNext() != QXmlStreamReader::EndElement)  { // reading closing tag STATE_HEX_HISTORY_ITEM
                            // qDebug() << tr("Was expecting closing tag %3, got %1[%2]").arg(reader->name().toString()).arg(reader->tokenString()).arg(bs->metaObject()->className());
                            break;
                        } else {
                           // qDebug() << tr("Next element read %1[%2]").arg(reader->name().toString()).arg(reader->tokenString());
                        }
                    }
                }

                if (bs->history.size() > 0) {
                    if (index < bs->history.size())
                        bs->currentHistoryPointer = index;
                    else
                        bs->currentHistoryPointer = (bs->history.isEmpty() ? 0 : bs->history.size() - 1);

                }
                if (readEndElement(GuiConst::STATE_HEX_HISTORY)) { // reading closing tag STATE_HEX_HISTORY
                    readEndElement(bs->metaObject()->className()); // read closing tag for classname
                }

            } else {
               // qDebug() << tr("No history saved");
            }

        }

    }
}

void ByteSourceStateObj::internalRun()
{
    qCritical() << tr("[%1] InternalRun not implemented T_T").arg(metaObject()->className());
}


ByteSourceClosingObj::ByteSourceClosingObj(ByteSourceAbstract *bs) :
    bs(bs)
{
    readonly = false;
    name = metaObject()->className();
}

ByteSourceClosingObj::~ByteSourceClosingObj()
{

}

void ByteSourceClosingObj::run()
{
    // last configuration bits for the bytesource
    if (!(flags & GuiConst::STATE_SAVE_REQUEST)) {
        bs->setReadOnly(readonly);
    }
}

void ByteSourceClosingObj::setReadonly(bool value)
{
    readonly = value;
}

