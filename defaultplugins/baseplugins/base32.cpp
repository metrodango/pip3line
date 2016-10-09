/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "base32.h"
#include "confgui/base32widget.h"

const QString Base32::id = "Base32";
const QStringList Base32::VARIATIONS = QStringList() << "RFC 4648" << "Crockford" << "Base32Hex (RFC 2938)";
const QByteArray Base32::CharTableRFC4648     = QByteArray("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");
const QByteArray Base32::CharTableCrockford   = QByteArray("0123456789ABCDEFGHJKMNPQRSTVWXYZ");
//const QByteArray Base32::CharTableZBase32     = QByteArray("ybndrfg8ejkmcpqxot1uwisza345h769");
const QByteArray Base32::CharTableBase32Hex   = QByteArray("0123456789ABCDEFGHIJKLMNOPQRSTUV");

Base32::Base32()
{
    includePadding = true;
    paddingChar = '=';
    variant = RFC4648;
}

Base32::~Base32()
{

}

QString Base32::name() const {
    return id;
}

QString Base32::description() const {
    return tr("Base32 encoding");
}

void Base32::transform(const QByteArray &input, QByteArray &output)
{
    output.clear();
    if (input.isEmpty())
        return;

    QByteArray charTable  = getCharTable();
    if (wayValue == TransformAbstract::INBOUND) {
        uchar current = 0;
        uchar newchar = 0;
        int bitsToRead = 0;
        int bitsRest = 0;
        int needMore = BlockSize;
        int index = 0;
        while (index < input.size() || bitsRest != 0) {
            if (bitsRest == 0) {
                newchar = (uchar)input.at(index);
                index++;
                bitsRest = ByteSize;
            } else {
                newchar <<= bitsToRead;
            }
            bitsToRead = qMin(needMore,bitsRest);
            if (needMore == BlockSize)
                current = (newchar >> (ByteSize - bitsToRead)) << (BlockSize - bitsToRead);
            else
                current = current | ((newchar >> (ByteSize - bitsToRead)));
            needMore = needMore - bitsToRead;
            bitsRest -= bitsToRead;
            if (needMore == 0) {
                output.append(charTable.at((uint)current));
                needMore = BlockSize;
                current = 0;
            }
        }

        if (needMore != BlockSize) {
            output.append(charTable.at((uint)current));
        }

        // padding
        if (includePadding) {
            int needpadding = (8 - (output.size() % 8)) % 8;
            for (int i = 0; i < needpadding; i++) {
                output.append(paddingChar);
            }
        }

    } else {
        QByteArray temp = input.toUpper();
        QByteArray invalid;

        while (temp.at(temp.size() - 1) == paddingChar)
            temp.chop(1);

        for (int i = 0; i < temp.size(); i++) {
            if (!charTable.contains(temp.at(i))) {
                if (!invalid.contains(temp.at(i)))
                    invalid.append(temp.at(i));
            }
        }
        if (!invalid.isEmpty())
            emit error(tr("Input string contains invalid Base32 character(s) : [%1] (they will be ignored)").arg(QString::fromUtf8(toPrintableString(invalid))),id);

        int index = 0;
        uchar current = 0;
        uchar newchar = (uchar)charTable.indexOf(temp.at(index));;
        int bitsRest = BlockSize;
        int needMore = ByteSize;

        while (index < temp.size() || bitsRest != 0) {

            int bitsToRead = qMin(needMore,bitsRest);

            current = current | ((newchar >> (bitsRest - bitsToRead) << (needMore - bitsToRead)));
            needMore = needMore - bitsToRead;
            bitsRest -= bitsToRead;
            if (needMore == 0) {
                output.append((char)current);
                needMore = ByteSize;
                current = 0;
            }

            if (bitsRest == 0) {
                while (++index < temp.size() && !charTable.contains(temp.at(index)));
                if (index >= temp.size())
                    break;
                newchar = (uchar)charTable.indexOf(temp.at(index));
                bitsRest = BlockSize;
            } else {
                newchar = (newchar << (ByteSize - bitsRest)) >> (ByteSize - bitsRest);
            }
        }

        if (bitsRest != 0) {
            output.append((char)current);
        }
    }
}

bool Base32::isTwoWays() {
    return true;
}

QHash<QString, QString> Base32::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLVARIANT,QString::number((int)variant));
    properties.insert(XMLPADDINGCHAR, saveChar(paddingChar));
    properties.insert(XMLINCLUDEPADDING,QString::number(includePadding ? 1 : 0));

    return properties;
}

bool Base32::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);

    bool ok = true;
    int val = 0;

    val = propertiesList.value(XMLVARIANT).toInt(&ok);
    if (!ok || (val != RFC4648 && val != CROCKFORD && val != BASE32HEX)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLVARIANT),id);
    } else {
        setVariant((CharSetVariant) val);
    }


    QString str = propertiesList.value(XMLPADDINGCHAR);
    char padtmp = '\x00';
    if (!loadChar(str,&padtmp)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLPADDINGCHAR),id);
    } else {
        res = setPaddingChar(padtmp) && res;
    }

    val = propertiesList.value(XMLINCLUDEPADDING).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLINCLUDEPADDING),id);
    } else {
        setIncludePadding(val == 1);
    }

    return res;
}

QWidget *Base32::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) Base32Widget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for Base32Widget X{");
    }
    return widget;
}

QString Base32::help() const
{
    QString help;
    help.append("<p>Base32 decoder/encoder</p><p>Available variants: ");
    help.append("<ul><li>RFC 4648</li> <li>Crockford</li><li>Base32hex (RFC 2938)</li></ul>");
    help.append("The default variant used is RFC 4648</p>");
    return help;
}

QByteArray Base32::getCharTable()
{
    switch (variant) {
        case RFC4648:
            return CharTableRFC4648;
        case CROCKFORD:
            return CharTableCrockford;
        case BASE32HEX:
            return CharTableBase32Hex;
        default:
            return CharTableRFC4648;
    }
}

bool Base32::isPaddingIncluded()
{
    return includePadding;
}

char Base32::getPaddingChar()
{
    return paddingChar;
}

Base32::CharSetVariant Base32::getVariant()
{
    return variant;
}

void Base32::setIncludePadding(bool val)
{
    if (includePadding != val) {
        includePadding = val;
        emit confUpdated();
    }
}

bool Base32::setPaddingChar(char val)
{
    if (getCharTable().contains(val)) {
        emit error(tr("Cannot use a character that is included in the charset as a padding character"),id);
        return false;
    }
    if (paddingChar != val) {
        paddingChar = val;
        emit confUpdated();
    }
    return true;
}

void Base32::setVariant(Base32::CharSetVariant val)
{
    if (variant != val) {
        variant = val;
        emit confUpdated();
    }
}

