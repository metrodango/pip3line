/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "iptranslateipv4.h"
#include <QHostAddress>
#include "confgui/iptranslateipv4widget.h"

const QString IPTranslateIPv4::id = "IPv4 translate";


IPTranslateIPv4::IPTranslateIPv4()
{
    base = 16;
    littleEndian = true;
    toUpper = true;
}

QString IPTranslateIPv4::name() const
{
    return id;
}

QString IPTranslateIPv4::description() const
{
    return "Translate IPv4 from/to unsigned int32 (base 10 or 16)";
}

void IPTranslateIPv4::transform(const QByteArray &input, QByteArray &output)
{
    bool ok = false;
    QHostAddress ip;

    if (wayValue == TransformAbstract::INBOUND) {
        quint32 num = input.toUInt(&ok,base);
        if (ok) {
            if (littleEndian)
                num = reverseBytes(num);
            ip.setAddress(num);
            output.append(ip.toString());
        } else {
            emit error(tr("Invalid unsigned integer value for an IPv4 address"),id);
        }
    } else {

        QString val = QString::fromUtf8(input);

        if (ip.setAddress(val)) {
            if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
                quint32 num = ip.toIPv4Address();
                if (littleEndian)
                    num = reverseBytes(num);
                output.append(QByteArray::number(num,base));
                if (toUpper)
                    output = output.toUpper();
            } else
                emit error(tr("Not an IPv4 address"),id);
        } else {
            emit error(tr("Invalid IPv4 address"),id);
        }
    }
}


bool IPTranslateIPv4::isTwoWays()
{
    return true;
}

QHash<QString, QString> IPTranslateIPv4::getConfiguration()
{
    QHash<QString, QString> properties = TransformAbstract::getConfiguration();
    properties.insert(XMLBASE,QString::number((int)base));
    properties.insert(XMLUPPERCASE,QString::number(toUpper ? 1 : 0));
    properties.insert(XMLLITTLEENDIAN,QString::number(littleEndian ? 1 : 0));

    return properties;
}

bool IPTranslateIPv4::setConfiguration(QHash<QString, QString> propertiesList)
{
    bool res = TransformAbstract::setConfiguration(propertiesList);
    bool ok;

    int val = propertiesList.value(XMLBASE).toInt(&ok);
    if (!ok) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLBASE),id);
    } else {
        setBase(val == 10 ? BASE10 : BASE16);
    }

    val = propertiesList.value(XMLUPPERCASE).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLUPPERCASE),id);
    } else {
        setUppercase(val ==  1);
    }

    val = propertiesList.value(XMLLITTLEENDIAN).toInt(&ok);
    if (!ok || (val != 0 && val != 1)) {
        res = false;
        emit error(tr("Invalid value for %1").arg(XMLLITTLEENDIAN),id);
    } else {
        setLittleEndian(val ==  1);
    }

    return res;
}

QWidget *IPTranslateIPv4::requestGui(QWidget *parent)
{
    QWidget * widget = new(std::nothrow) IPTranslateIPv4Widget(this, parent);
    if (widget == nullptr) {
        qFatal("Cannot allocate memory for IPTranslateIPv4Widget X{");
    }
    return widget;
}

QString IPTranslateIPv4::inboundString() const
{
    return tr("int32 to IP");
}

QString IPTranslateIPv4::outboundString() const
{
    return tr("IP to int32");
}

QString IPTranslateIPv4::help() const
{
    QString help;
    help.append("<p>Translate IPv4 from/to unsigned int32 (base 10 or 16)</p><p>For example: 10.128.128.2 => 41975818 or in hex 280800A<br>There is an option to precise the endianess.</p>");
    return help;
}

void IPTranslateIPv4::setBase(IPTranslateIPv4::BASE val)
{
    if (val == BASE10)
        base = 10;
    else
        base = 16;
    emit confUpdated();
}

IPTranslateIPv4::BASE IPTranslateIPv4::getBase() const
{
    if (base == 10) {
        return BASE10;
    } else {
        return BASE16;
    }
}

void IPTranslateIPv4::setLittleEndian(bool val)
{
    littleEndian = val;
    emit confUpdated();
}

bool IPTranslateIPv4::isLittleendian() const
{
    return littleEndian;
}

void IPTranslateIPv4::setUppercase(bool val)
{
    toUpper = val;
    emit confUpdated();
}

bool IPTranslateIPv4::isFormatUpperCase() const
{
    return toUpper;
}

quint32 IPTranslateIPv4::reverseBytes(quint32 val)
{
    return   (((val & 0xFF000000) >> 24) | \
              ((val & 0x00FF0000) >> 8) | \
              ((val & 0x0000FF00) << 8)  | \
              ((val & 0x000000FF) << 24));
}
