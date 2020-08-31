#include "filteritem.h"
#include "shared/guiconst.h"
#include <QDebug>
#include <QTimeZone>
#include "packetmodelabstract.h"

const QRegExp FilterItem::nameRegexp = QRegExp("^[a-z0-9]{1,20}$",Qt::CaseInsensitive);

FilterItem::FilterItem() :
    QObject(nullptr),
    reverseSelection(false)
{
    name = "INVALID";
    filterId = FilterItem::INVALID;
}

bool FilterItem::operator ==(const FilterItem &other) const
{
    return this->name == other.name;
}

FilterItem::~FilterItem()
{

}

QString FilterItem::getName() const
{
    return name;
}

void FilterItem::setName(const QString &nname)
{
    if (nameRegexp.exactMatch(nname)) {
        QString temp = nname.toUpper();
        if (!(temp == "NOT" || temp == "OR" || temp == "AND" || temp == "XOR"))
            name = nname;
    }
}

bool FilterItem::isReverseSelection() const
{
    return reverseSelection;
}

void FilterItem::setReverseSelection(bool reverse)
{
    reverseSelection = reverse;
}

bool FilterItem::selectable(QSharedPointer<Packet> )
{
    return false;
}

bool FilterItem::isValid()
{
    return !name.isEmpty();
}

QHash<QString, QString> FilterItem::getConfiguration()
{
    QHash<QString, QString> ret;
    ret.insert(GuiConst::STATE_NAME, name);
    ret.insert(GuiConst::STATE_REVERSE_SELECTION, reverseSelection ? GuiConst::STATE_YES : GuiConst::STATE_NO);
    ret.insert(GuiConst::STATE_FILTER_ID, QString::number(filterId));
    return ret;
}

void FilterItem::setConfiguration(const QHash<QString, QString> &conf)
{
    if (conf.contains(GuiConst::STATE_NAME)) {
        setName(conf.value(GuiConst::STATE_NAME));
    }

    if (conf.contains(GuiConst::STATE_REVERSE_SELECTION)) {
        reverseSelection = conf.value(GuiConst::STATE_REVERSE_SELECTION) == GuiConst::STATE_YES;
    }
}

FilterItem *FilterItem::instanciateFromConf(const QHash<QString, QString> &conf)
{
    FilterItem * ret = nullptr;

    if (conf.contains(GuiConst::STATE_FILTER_ID)) {
        bool ok = false;
        int val = conf.value(GuiConst::STATE_FILTER_ID, QString::number(INVALID)).toInt(&ok);
        if (ok) {
            switch (val) {
                case Data:
                    ret = new(std::nothrow) FilterData();
                    break;
                case TimeStamp:
                    ret = new(std::nothrow) FilterTimeStamp();
                    break;
                case Length:
                    ret = new(std::nothrow) FilterLength();
                    break;
                case Direction:
                    ret = new(std::nothrow) FilterDirection();
                    break;
                case Text:
                    ret = new(std::nothrow) FilterText();
                    break;
                case CID:
                    ret = new(std::nothrow) FilterCIDs();
                    break;
                default:
                    qCritical() << QObject::tr("[FilterItem::instanciateFromConf] Invalid Filter ID");
            }

            if (ret != nullptr) {
                ret->setConfiguration(conf);

                if (!ret->isValid()) {
                    delete ret;
                    ret = nullptr;
                }
            }
        }

    } else {
        qCritical() << QObject::tr("[FilterItem::instanciateFromConf] Filter ID not found");
    }

    return ret;
}

FilterItem::FilterType FilterItem::getFilterId() const
{
    return filterId;
}

FilterItemsList::FilterItemsList()
{

}

FilterItemsList::FilterItemsList(const FilterItemsList &other) :
    QList<QSharedPointer<FilterItem> > (other)
{

}

FilterItemsList &FilterItemsList::operator =(const FilterItemsList &other)
{
    QList<QSharedPointer<FilterItem> >::operator =(other);

    return *this;
}

FilterItemsList::~FilterItemsList()
{

}

QSharedPointer<FilterItem> FilterItemsList::getFilter(const QString &name)
{
    QSharedPointer<FilterItem> ret;
    for (int i = 0; i < size(); i++) {
        if (at(i)->getName() == name) {
            ret = at(i);
            break;
        }
    }

    return ret;
}

bool FilterItemsList::containsFilter(const QString &name)
{
    bool ret = false;
    for (int i = 0; i < size(); i++) {
        if (at(i)->getName() == name) {
            ret = true;
            break;
        }
    }

    return ret;
}

FilterData::FilterData() :
    startingOffset(0),
    mask(nullptr),
    searchType(FilterData::HEXA)
{
    filterId = FilterItem::Data;
}

FilterData::~FilterData()
{
    if (mask != nullptr)
        delete[] mask;
}


QByteArray FilterData::getSearchValue() const
{
    return searchValue;
}

void FilterData::setSearchValue(const QByteArray &nvalue)
{
    searchValue = nvalue;
    if (mask != nullptr)
        delete[] mask;

    int msize = searchValue.size();
    if (msize > 0) {
        mask = new(std::nothrow) char[static_cast<size_t>(msize)];
        memset(mask, 0,static_cast<size_t>(msize));

        for (int i = 0; i < msize;i++) {
            mask[i] = '\xFF'; // no widlcard by default
        }
    } else {
        mask = nullptr;
    }
}

int FilterData::getStartingOffset() const
{
    return startingOffset;
}

void FilterData::setStartingOffset(const int &offset)
{
    if (offset >= 0)
        startingOffset = offset;
    else {
        startingOffset = 0;
        qWarning() << "[FilterItem::setStartingOffset] offset is a negative number, setting to 0";
    }
}

QString FilterData::getHexValueWithMask() const
{
    QString ret;

    for (int i = 0; i < searchValue.size(); i++) {
        if (mask[i] == '\xFF') {
            ret.append(QString::fromUtf8(searchValue.mid(i,1).toHex()));
        } else {
            ret.append("**");
        }
    }

    return ret;
}

bool FilterData::selectable(QSharedPointer<Packet> packet)
{
    QByteArray data = packet->getData();
    int length = data.size();
    bool ret = false;
    int searchSize = searchValue.size();
    int endofSearch = length - searchSize;
    if (startingOffset >= length)  // if offset starts after the data, return false
        return ret;

    if (endofSearch <= 0 )  // if the searched value is bigger than the searched item, return false
        return ret;

    int offset, i;
    for (offset = startingOffset; offset < endofSearch; offset++) {
        for (i = 0; i < searchSize; i++) {
            if ((data[offset + i] & mask[i]) != searchValue[i])
                break;
        }
        if (i < searchSize) {
            continue; // no luck with this window, move to next one
        } else {
            ret = true;
        }
    }

    return reverseSelection ? !ret : ret;
}

QHash<QString, QString> FilterData::getConfiguration()
{
    QHash<QString, QString> ret = FilterItem::getConfiguration();
    ret.insert(GuiConst::STATE_DATA, QString::fromUtf8(searchValue.toBase64()));
    ret.insert(GuiConst::STATE_OFFSET, QString::number(startingOffset));

    if (mask != nullptr) {
        int length = searchValue.size(); // mask has to be the same size as the search value
        QString maskstr;
        for (int i = 0; i < length; i++) {
            maskstr.append(mask[i] ? QString("1") : QString("0"));
        }

        if (!maskstr.isEmpty()) {
            ret.insert(GuiConst::STATE_MASK, maskstr);
        }
    }
    ret.insert(GuiConst::STATE_TYPE, QString::number(static_cast<int>(searchType)));
    return ret;
}

void FilterData::setConfiguration(const QHash<QString, QString> &conf)
{
    FilterItem::setConfiguration(conf);

    if (conf.contains(GuiConst::STATE_MASK)) {
        QString maskstr = conf.value(GuiConst::STATE_MASK);
        QBitArray maskBits;
        maskBits.resize(maskstr.size());
        for (int i = 0; i < maskstr.size(); i++) {
            maskBits.setBit(i, maskstr.at(i) == QChar('1'));
        }

        setBitMask(maskBits);
    }

    if (conf.contains(GuiConst::STATE_DATA)) {
        setSearchValue(QByteArray::fromBase64(conf.value(GuiConst::STATE_DATA).toUtf8()));
    }

    bool ok = false;

    if (conf.contains(GuiConst::STATE_OFFSET)) {
        int val = conf.value(GuiConst::STATE_OFFSET).toInt(&ok);
        if (ok && val >= 0)
            startingOffset = val;
    }

    if (conf.contains(GuiConst::STATE_TYPE)) {
        int val = conf.value(GuiConst::STATE_TYPE).toInt(&ok);
        if (ok && (val == HEXA || val == UTF8 || val == UTF16))
            searchType = static_cast<FilterData::SEARCH_TYPE>(val);
    }
}

bool FilterData::isValid()
{
    return FilterItem::isValid() && !searchValue.isEmpty();
}

FilterData::SEARCH_TYPE FilterData::getSearchType() const
{
    return searchType;
}

void FilterData::setSearchType(const FilterData::SEARCH_TYPE &type)
{
    searchType = type;
}

void FilterData::setBitMask(QBitArray bitMask)
{
    int msize = searchValue.size();

    if (msize <= 0) // nothing to see here
        return;

    if (bitMask.size() < msize) { // pad the mask array with true values
        int index = bitMask.isEmpty() ? 0 : bitMask.size() - 1;
        bitMask.resize(msize);
        for (; index < msize; index++)
            bitMask.setBit(index);
    }

    if (mask != nullptr)
        delete[] mask;

    mask = new(std::nothrow) char[static_cast<size_t>(msize)];
    memset(mask, 0,static_cast<size_t>(msize));

    for (int i = 0; i < msize;i++) {
        mask[i] = bitMask.testBit(i) ? '\xFF' : '\x00';
    }
}

FilterTimeStamp::FilterTimeStamp()
{
    filterId = FilterItem::TimeStamp;
}

FilterTimeStamp::~FilterTimeStamp()
{

}

bool FilterTimeStamp::selectable(QSharedPointer<Packet> packet)
{
    if (!after.isValid() && !before.isValid())
        return false;

    bool ret = true;
    if (after.isValid()) {
        ret  = after <= packet->getTimestamp();
    }

    if (before.isValid()) {
        ret = ret && (packet->getTimestamp() <= before);
    }

    return reverseSelection ? !ret : ret;
}

QHash<QString, QString> FilterTimeStamp::getConfiguration()
{
    QHash<QString, QString> ret = FilterItem::getConfiguration();
    QTimeZone localtz;
    if (before.isValid()) {
        ret.insert(GuiConst::STATE_BEFORE, QString::number(before.toUTC().toMSecsSinceEpoch()));
        localtz = before.timeZone();
    }

    if (after.isValid()) {
        ret.insert(GuiConst::STATE_AFTER, QString::number(after.toUTC().toMSecsSinceEpoch()));
        if (!localtz.isValid()) { // we already got it from "before". yes, "before" override "after"
            localtz = after.timeZone();
        }
    }
    if (localtz.isValid())
        ret.insert(GuiConst::STATE_TIMEZONE, QString::fromUtf8(localtz.id().toBase64()));
    return ret;
}

void FilterTimeStamp::setConfiguration(const QHash<QString, QString> &conf)
{
    FilterItem::setConfiguration(conf);

    bool ok = false;

    if (conf.contains(GuiConst::STATE_BEFORE)) {
        qint64 val = conf.value(GuiConst::STATE_BEFORE).toLongLong(&ok);
        if (ok) {
            before.setTimeZone(QTimeZone::utc());
            before.setMSecsSinceEpoch(val);
        }
    }

    if (conf.contains(GuiConst::STATE_AFTER)) {
        qint64 val = conf.value(GuiConst::STATE_AFTER).toLongLong(&ok);
        if (ok) {
            after.setTimeZone(QTimeZone::utc());
            after.setMSecsSinceEpoch(val);
        }
    }

    if (conf.contains(GuiConst::STATE_TIMEZONE)) {
        QByteArray tz = QByteArray::fromBase64(conf.value(GuiConst::STATE_AFTER).toUtf8());
        QTimeZone localtz(tz);
        if (localtz.isValid()) {
            if (before.isValid())
                before.setTimeZone(localtz);
            if (after.isValid())
                after.setTimeZone(localtz);
        }
    }
}

bool FilterTimeStamp::isValid()
{
    return FilterItem::isValid() && (after.isValid() || before.isValid());
}

QDateTime FilterTimeStamp::getAfter() const
{
    return after;
}

void FilterTimeStamp::setAfter(const QDateTime &value)
{
    after = value;
}

QDateTime FilterTimeStamp::getBefore() const
{
    return before;
}

void FilterTimeStamp::setBefore(const QDateTime &value)
{
    before = value;
}

const QStringList FilterLength::OperatorsString = QStringList() << "<" << "<=" << ">" << ">=" << "=" << "!=";

FilterLength::FilterLength() :
    op(INVALID)
{
    filterId = FilterItem::Length;
    lengthValue = 0;
    op = FilterLength::INVALID;
}

FilterLength::~FilterLength()
{

}

bool FilterLength::selectable(QSharedPointer<Packet> packet)
{
    bool ret = false;
    int length = packet->getData().size();
    switch (op) {
        case LESS:
            ret = length < lengthValue;
            break;
        case LESSEQUAL:
            ret = length <= lengthValue;
            break;
        case GREATER:
            ret = length > lengthValue;
            break;
        case GREATEREQUAL:
            ret = length >= lengthValue;
            break;
        case EQUAL:
            ret = length == lengthValue;
            break;
        case DIFFERENT:
            ret = length != lengthValue;
            break;
        default:
            qCritical() << tr("[FilterLength::selectable] invalid operation");
    }

    return reverseSelection ? !ret : ret;
}

QHash<QString, QString> FilterLength::getConfiguration()
{
    QHash<QString, QString> ret = FilterItem::getConfiguration();

    ret.insert(GuiConst::STATE_SIZE, QString::number(lengthValue));
    ret.insert(GuiConst::STATE_TYPE, QString::number(op));
    return ret;
}

void FilterLength::setConfiguration(const QHash<QString, QString> &conf)
{
    FilterItem::setConfiguration(conf);

    bool ok = false;

    if (conf.contains(GuiConst::STATE_SIZE)) {
        int val = conf.value(GuiConst::STATE_SIZE).toInt(&ok);
        if (ok && val >= 0)
            lengthValue = val;
    }

    if (conf.contains(GuiConst::STATE_TYPE)) {
        int val = conf.value(GuiConst::STATE_TYPE).toInt(&ok);
        if (ok && (val == LESS ||
                   val == LESSEQUAL ||
                   val == GREATER ||
                   val == GREATEREQUAL ||
                   val == EQUAL ||
                   val == DIFFERENT))

            op = static_cast<FilterLength::Operators>(val);
    }
}

bool FilterLength::isValid()
{
    return FilterItem::isValid() && op != FilterLength::INVALID;
}

int FilterLength::getLengthValue() const
{
    return lengthValue;
}

void FilterLength::setLengthValue(int value)
{
    lengthValue = value;
}

FilterLength::Operators FilterLength::getOp() const
{
    return op;
}

void FilterLength::setOp(const FilterLength::Operators &value)
{
    op = value;
}

FilterDirection::FilterDirection()
{
    filterId = FilterItem::Direction;
    direction = Packet::NODIRECTION;
}

FilterDirection::~FilterDirection()
{

}

bool FilterDirection::selectable(QSharedPointer<Packet> packet)
{
    bool ret = (packet->getDirection() == direction);
    return reverseSelection ? !ret : ret;
}

QHash<QString, QString> FilterDirection::getConfiguration()
{
    QHash<QString, QString> ret = FilterItem::getConfiguration();

    ret.insert(GuiConst::STATE_DIRECTION, QString::number(direction));
    return ret;
}

void FilterDirection::setConfiguration(const QHash<QString, QString> &conf)
{
    FilterItem::setConfiguration(conf);

    bool ok = false;

    if (conf.contains(GuiConst::STATE_DIRECTION)) {
        int val = conf.value(GuiConst::STATE_SIZE).toInt(&ok);
        if (ok && (val==Packet::LEFTRIGHT || val==Packet::RIGHTLEFT || val==Packet::NODIRECTION ))
            direction = static_cast<Packet::Direction>(val);
    }
}

bool FilterDirection::isValid()
{
    return FilterItem::isValid();
}

Packet::Direction FilterDirection::getDirection() const
{
    return direction;
}

void FilterDirection::setDirection(const Packet::Direction &value)
{
    direction = value;
}

const QStringList FilterText::RegexpTypesStr = QStringList() << "Regexp"
                                                             << "wildcard"
                                                             << "fixed"
                                                             << "Regexp (greedy)"
                                                             << "unix shell wildcard"
                                                             << "W3 Xml Schema 11";

FilterText::FilterText()
{
    filterId = FilterItem::Text;
}

FilterText::~FilterText()
{

}

bool FilterText::selectable(QSharedPointer<Packet> packet)
{
    bool ret = false;

    if (targetColumn == PacketModelAbstract::COLUMN_COMMENT_STR) {
        QString val = packet->getComment();
        ret = val.contains(regexp);
    } else {
        QHash<QString, QString> fields = packet->getAdditionalFields();
        if (fields.contains(targetColumn)) {
            QString val = fields.value(targetColumn);
            ret = val.contains(regexp);
        }
    }

    return reverseSelection ? !ret : ret;
}

QHash<QString, QString> FilterText::getConfiguration()
{
    QHash<QString, QString> ret = FilterItem::getConfiguration();

    ret.insert(GuiConst::STATE_REGEXP_PATTERN, regexp.pattern());
    ret.insert(GuiConst::STATE_REGEXP_PATTERN_SYNTAX, QString::number(regexp.patternSyntax()));
    ret.insert(GuiConst::STATE_REGEXP_CASE_SENSITIVE, QString::number(regexp.caseSensitivity()));
    ret.insert(GuiConst::STATE_COLUMN, targetColumn);
    return ret;
}

void FilterText::setConfiguration(const QHash<QString, QString> &conf)
{
    FilterItem::setConfiguration(conf);

    bool ok = false;

    if (conf.contains(GuiConst::STATE_REGEXP_PATTERN)) {
        regexp.setPattern(conf.value(GuiConst::STATE_REGEXP_PATTERN));
    }

    if (conf.contains(GuiConst::STATE_REGEXP_PATTERN_SYNTAX)) {
        int val = conf.value(GuiConst::STATE_REGEXP_PATTERN_SYNTAX).toInt(&ok);
        if (ok && (val==QRegExp::RegExp ||
                   val==QRegExp::Wildcard ||
                   val==QRegExp::FixedString ||
                   val==QRegExp::RegExp2 ||
                   val==QRegExp::WildcardUnix ||
                   val==QRegExp::W3CXmlSchema11 ))
            regexp.setPatternSyntax(static_cast<QRegExp::PatternSyntax>(val));
    }

    if (conf.contains(GuiConst::STATE_REGEXP_CASE_SENSITIVE)) {
        int val = conf.value(GuiConst::STATE_REGEXP_CASE_SENSITIVE).toInt(&ok);
        if (ok && (val==Qt::CaseSensitive ||
                   val==Qt::CaseInsensitive ))
            regexp.setCaseSensitivity(static_cast<Qt::CaseSensitivity>(val));
    }

    if (conf.contains(GuiConst::STATE_COLUMN)) {
        targetColumn = conf.value(GuiConst::STATE_COLUMN);
    }
}

bool FilterText::isValid()
{
    return FilterItem::isValid() && regexp.isValid() && !targetColumn.isEmpty();
}

QRegExp FilterText::getRegexp() const
{
    return regexp;
}

void FilterText::setRegexp(const QRegExp &value)
{
    regexp = value;
}

QString FilterText::getTargetColumn() const
{
    return targetColumn;
}

void FilterText::setTargetColumn(QString columnName)
{
    targetColumn = columnName;
}

const QString FilterCIDs::SEP = ",";

FilterCIDs::FilterCIDs()
{
    filterId = FilterItem::CID;
}

FilterCIDs::~FilterCIDs()
{

}

bool FilterCIDs::selectable(QSharedPointer<Packet> packet)
{
    bool ret = cidList.contains(packet->getSourceid());
    return reverseSelection ? !ret : ret;
}

QHash<QString, QString> FilterCIDs::getConfiguration()
{
    QHash<QString, QString> ret = FilterItem::getConfiguration();

    QStringList flist;
    for (int i = 0; i < cidList.size(); i++) {
        flist.append(QString::number(cidList.at(i)));
    }
    ret.insert(GuiConst::STATE_CID_LIST,flist.join(FilterCIDs::SEP));
    return ret;
}

void FilterCIDs::setConfiguration(const QHash<QString, QString> &conf)
{
    FilterItem::setConfiguration(conf);
    cidList.clear();
    bool ok  = false;

    if (conf.contains(GuiConst::STATE_CID_LIST)) {
        QStringList list = conf.value(GuiConst::STATE_CID_LIST).split(FilterCIDs::SEP,
                                                              #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
                                                                      Qt::SkipEmptyParts
                                                              #else
                                                                      QString::SkipEmptyParts
                                                              #endif
                                                                      );
        for (int i = 0; i < list.size(); i++) {
            int val = list.at(i).toInt(&ok);
            if (ok) {
                cidList.append(val);
            }
        }
    }
}

bool FilterCIDs::isValid()
{
    return FilterItem::isValid() && !cidList.isEmpty();
}

QList<int> FilterCIDs::getCidList() const
{
    return cidList;
}

void FilterCIDs::setCidList(const QList<int> &value)
{
    cidList = value;
}
