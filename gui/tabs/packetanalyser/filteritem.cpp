#include "filteritem.h"
#include "shared/guiconst.h"
#include <QDebug>

FilterItem::FilterItem(QObject *parent) :
    QObject(parent),
    startingOffset(0),
    reverseSelection(false),
    mask(nullptr),
    searchType(FilterItem::HEXA)
{

}

FilterItem::FilterItem(const FilterItem &other) :
    QObject(other.parent())
{
    // usefull for the copy constructor
    mask = nullptr;
    *this = other;
}

FilterItem &FilterItem::operator =(const FilterItem &other)
{
    if (&other != this) {
        this->name = other.name;
        this->value = other.value;
        this->startingOffset = other.startingOffset;
        this->reverseSelection = other.reverseSelection;
        this->searchType = other.searchType;
        if (mask != nullptr)
            delete[] mask;

        mask = nullptr;

        int msize = value.size();

        if (value.size() > 0) {
            mask = new(std::nothrow) char[msize];
            memset(mask, 0,msize);

            for (int i = 0; i < msize;i++) {
                mask[i] = other.mask[i];
            }
        }
    }

    return *this;
}

bool FilterItem::operator ==(const FilterItem &other) const
{
    return this->name == other.name;
}

FilterItem::~FilterItem()
{
    if (mask != nullptr)
        delete[] mask;
}

QString FilterItem::getName() const
{
    return name;
}

void FilterItem::setName(const QString &value)
{
    name = value;
}

QByteArray FilterItem::getValue() const
{
    return value;
}

void FilterItem::setValue(const QByteArray &nvalue)
{
    value = nvalue;
    if (mask != nullptr)
        delete[] mask;

    int msize = value.size();
    if (msize > 0) {
        mask = new(std::nothrow) char[msize];
        memset(mask, 0,msize);

        for (int i = 0; i < msize;i++) {
            mask[i] = '\xFF'; // no widlcard by default
        }
    } else {
        mask = nullptr;
    }
}

int FilterItem::getStartingOffset() const
{
    return startingOffset;
}

void FilterItem::setStartingOffset(const int &value)
{
    if (value > 0)
        startingOffset = value;
    else {
        startingOffset = 0;
        qCritical() << "offset is a negative number, setting to 0";
    }
}

bool FilterItem::isReverseSelection() const
{
    return reverseSelection;
}

void FilterItem::setReverseSelection(bool value)
{
    reverseSelection = value;
}

void FilterItem::setBitMask(QBitArray bitMask)
{
    int msize = value.size();

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

    mask = new(std::nothrow) char[msize];
    memset(mask, 0,msize);

    for (int i = 0; i < msize;i++) {
        mask[i] = bitMask.testBit(i) ? '\xFF' : '\x00';
    }
}

bool FilterItem::selectable(QByteArray data)
{
    int length = data.size();
    bool ret = false;
    int searchSize = value.size();
    int endofSearch = length - searchSize;
    if (startingOffset >= length)  // if offset starts after the data, return false
        return ret;

    if (endofSearch <= 0 )  // if the searched value is bigger than the searched item, return false
        return ret;

    int offset, i;
    for (offset = startingOffset; offset < endofSearch; offset++) {
        for (i = 0; i < searchSize; i++) {
            if ((data[i] & mask[i]) != value[i])
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

FilterItem::SEARCH_TYPE FilterItem::getSearchType() const
{
    return searchType;
}

void FilterItem::setSearchType(const FilterItem::SEARCH_TYPE &value)
{
    searchType = value;
}

bool FilterItem::isValid()
{
    return !name.isEmpty() && !value.isEmpty();
}

QString FilterItem::getHexValueWithMask() const
{
    QString ret;

    for (int i = 0; i < value.size(); i++) {
        if (mask[i] == '\xFF') {
            ret.append(QString::fromUtf8(value.mid(i,1).toHex()));
        } else {
            ret.append("**");
        }
    }

    return ret;
}

QHash<QString, QString> FilterItem::getConfiguration()
{
    QHash<QString, QString> ret;
    ret.insert(GuiConst::STATE_NAME, name);
    ret.insert(GuiConst::STATE_DATA, QString::fromUtf8(value.toBase64()));
    ret.insert(GuiConst::STATE_OFFSET, QString::number(startingOffset));
    ret.insert(GuiConst::STATE_REVERSE_SELECTION, reverseSelection ? GuiConst::STATE_YES : GuiConst::STATE_NO);
    if (mask != nullptr) {
        int length = value.size(); // mask has to be the same size as the search value
        QString maskstr;
        for (int i = 0; i < length; i++) {
            maskstr.append(mask[i] ? QString("1") : QString("0"));
        }

        if (!maskstr.isEmpty()) {
            ret.insert(GuiConst::STATE_MASK, maskstr);
        }
    }
    ret.insert(GuiConst::STATE_TYPE, QString::number((int)searchType));
    return ret;
}

void FilterItem::setConfiguration(const QHash<QString, QString> &conf)
{
    bool ok = false;
    if (conf.contains(GuiConst::STATE_NAME)) {
        name = conf.value(GuiConst::STATE_NAME);
    }

    if (conf.contains(GuiConst::STATE_DATA)) {
        setValue(QByteArray::fromBase64(conf.value(GuiConst::STATE_DATA).toUtf8()));
    }

    if (conf.contains(GuiConst::STATE_OFFSET)) {
        int val = conf.value(GuiConst::STATE_OFFSET).toInt(&ok);
        if (ok && val >= 0)
            startingOffset = val;
    }

    if (conf.contains(GuiConst::STATE_REVERSE_SELECTION)) {
        reverseSelection = conf.value(GuiConst::STATE_REVERSE_SELECTION) == GuiConst::STATE_YES;
    }

    if (conf.contains(GuiConst::STATE_MASK)) {
        QString maskstr = conf.value(GuiConst::STATE_MASK);
        QBitArray maskBits;
        maskBits.resize(maskstr.size());
        for (int i = 0; i < maskstr.size(); i++) {
            maskBits.setBit(i, maskstr.at(i) == QChar('1'));
        }

        setBitMask(maskBits);
    }

    if (conf.contains(GuiConst::STATE_TYPE)) {
        int val = conf.value(GuiConst::STATE_TYPE).toInt(&ok);
        if (ok && (val == (int)HEXA || val == (int)UTF8 || val == (int)UTF16))
            searchType = (FilterItem::SEARCH_TYPE)val;
    }
}
