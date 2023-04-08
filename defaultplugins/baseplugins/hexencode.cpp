/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "hexencode.h"
#include "confgui/hexencodewidget.h"
#include <QDebug>

const QString HexEncode::id = "Hexadecimal";
const QString HexEncode::XML_ADDPREFIX = "AddHexPrefix";
HexEncode::HexEncode()
{
    type = NORMAL;
    addHexPrefix = false;
}

HexEncode::~HexEncode()
{
}

QString HexEncode::name() const {
    return id;
}

QString HexEncode::description() const {
    return tr("Char hexadecimal encoding");
}

bool HexEncode::isTwoWays() {
    return true;
}

QHash<QString, QString> HexEncode::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLTYPE,QString::number(type));
    properties.insert(XML_ADDPREFIX, QString::number(addHexPrefix ? 1 : 0));
    return properties;
}

bool HexEncode::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok = false;

    int val = propertiesList.value(XMLTYPE).toInt(&ok);
    if (!ok || (val != NORMAL && val != ESCAPED_MIXED && val != ESCAPED && val != CSTYLE && val != CSV)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLTYPE),id);
    } else {
        setType(static_cast<Type>(val));
    }

    val = propertiesList.value(XML_ADDPREFIX).toInt(&ok);
    setAddHexPrefix(val == 1); // no advanced check here for backward compatibility

    return res;
}

QWidget *HexEncode::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) HexEncodeWidget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for HexEncodeWidget X{");
    }
    return widget;
}

QString HexEncode::help() const
{
    QString help;
    help.append("<p>Hexadecimal encoder</p>");
    help.append("<p>It is possible to choose a specific output format.");
    help.append("<br>Currently available: <ul>");
    help.append("<li>Basic hexadecimal </li>");
    help.append("<li>Escaped Hexadecimal (i.e. \"\\xAD\")</li>");
    help.append("<li>Escaped Hexadecimal Mixed (i.e. \"abc123\\xAD\")*</li>");
    help.append("<li>C-Style array (i.e. \"{ 0xAD, 0x01 }\" )</li>");
    help.append("<li>CSV format, i.e. \"AD, 01\"</li></ul>");
    help.append("* Escaped Hexadecimal Mixed is encoding only non-printable characters, and while decoding just append any incorrect value to the output (instead of discarding it)");
    help.append("</p>");
    return help;
}

void HexEncode::transform(const QByteArray &input, QByteArray &output) {
    output.clear();
    if (input.isEmpty())
        return;
    QByteArray temp;
    int i = 0;
    if (wayValue == TransformAbstract::INBOUND) {
        temp = input.toHex();
        switch (type) {
            case NORMAL:
                output = temp;
                if (addHexPrefix) {
                    output.prepend("0x");
                }
                break;
            case ESCAPED_MIXED:
                for (i = 0; i < input.size(); i += 1) {
                    char c = input.at(i);
                    if (c < '\x20' || c > '\x7e')
                        output.append(QByteArray("\\x").append(QByteArray(1,c).toHex()));
                    else
                        output.append(c);
                }
                break;
            case ESCAPED:
                for (i = 0; i < temp.size(); i += 2) {
                    output.append(QString("\\x%1%2").arg(temp.at(i)).arg(temp.at(i+1)).toUtf8());
                }
                break;
            case CSTYLE:
                output.append("{");
                for (i = 0; i < temp.size(); i += 2) {
                    output.append(QString(" 0x%1%2,").arg(temp.at(i)).arg(temp.at(i+1)).toUtf8());
                }
                output.chop(1);
                output.append(" }");
                break;
            case CSV:
                for (i = 0; i < temp.size(); i += 2) {
                    output.append(QString("%1%2,").arg(temp.at(i)).arg(temp.at(i+1)).toUtf8());
                }
                break;
        }
    }
    else {
        QString HEXCHAR("abcdefABCDEF0123456789");

        switch (type) {
            case NORMAL:
                output = fromHex(input);
                break;
            case ESCAPED_MIXED:
                while (i < input.size()) {
                    if (input.at(i) == '\\' && i < input.size() - 3 &&  input.at(i+1) == 'x') {
                        if (HEXCHAR.contains(input.at(i+2)) && HEXCHAR.contains(input.at(i+3))) {
                            temp.append(QByteArray::fromHex(QByteArray(1,input.at(i+2)).append(input.at(i+3))));
                            i += 4;
                            continue;
                        }
                    }
                    temp.append(input.at(i));
                    i++;
                }
                output = temp;
                break;
            case ESCAPED:
                while (i < input.size()) {
                    if (input.at(i) == '\\' && i < input.size() - 3 &&  input.at(i+1) == 'x') {
                        if (HEXCHAR.contains(input.at(i+2)) && HEXCHAR.contains(input.at(i+3))) {
                            temp.append(input.at(i+2)).append(input.at(i+3));
                            i += 4;
                            continue;
                        }
                    }
                    i++;
                }
                output = fromHex(temp);
                break;
            case CSTYLE:
                while (i < input.size()) {
                    if (input.at(i) == '0' && i < input.size() - 3 &&  input.at(i+1) == 'x') {
                        if (HEXCHAR.contains(input.at(i+2)) && HEXCHAR.contains(input.at(i+3))) {
                            temp.append(input.at(i+2)).append(input.at(i+3));
                            i += 4;
                            continue;
                        }
                    }
                    i++;
                }
                output = fromHex(temp);
                break;
            case CSV:
                QList<QByteArray> list = input.split(',');
                for (i = 0; i < list.size(); i++) {
                    temp.append(list.at(i).trimmed());
                }
                output = fromHex(temp);
                break;
        }
    }
}

HexEncode::Type HexEncode::getType()
{
    return type;
}

void HexEncode::setType(HexEncode::Type ntype)
{
    if (type != ntype) {
        type = ntype;
        emit confUpdated();
    }
}
bool HexEncode::getAddHexPrefix() const
{
    return addHexPrefix;
}

void HexEncode::setAddHexPrefix(bool value)
{
    if (addHexPrefix != value) {
        addHexPrefix = value;
        emit confUpdated();
    }
}

