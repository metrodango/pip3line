/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#include "networkmaskipv4.h"
#include <QHostAddress>

const QString NetworkMaskIPv4::id = "Network mask IPv4";

NetworkMaskIPv4::NetworkMaskIPv4()
{
}

QString NetworkMaskIPv4::name() const
{
    return id;
}

QString NetworkMaskIPv4::description() const
{
    return tr("Translate network mask IPv4 number to an IPv4 string");
}

void NetworkMaskIPv4::transform(const QByteArray &input, QByteArray &output)
{
    bool ok = false;
    QHostAddress ip;
    if (wayValue == TransformAbstract::INBOUND) {
        int num = input.toInt(&ok,10);
        if (ok && num >= 0 && num < 33 ) {
            quint32 ipVal = 0;
            if (num == 0)
                ipVal = 0;
            else if (num == 32)
                ipVal = 0xFFFFFFFF;
            else
                ipVal  = 0xFFFFFFFF << (32 - num);
            ip.setAddress(ipVal);
            output.append(ip.toString());
        } else {
            emit error(tr("Not a valid IPv4 netmask value"),id);
        }
    } else {
        QString val = QString::fromUtf8(input);

        if (ip.setAddress(val)) {
            int netmask = 0;
            if (ip.protocol() == QAbstractSocket::IPv4Protocol) {
                quint32 num = ip.toIPv4Address();
                int zeros = 0;
                int cur = 0;
                for (; cur < 32; cur ++) {
                    if (((num >> cur) & 0x00000001) == 0)
                        zeros++;
                    else
                        break;
                }
                for (; cur < 32; cur++)
                    if (((num >> cur) & 0x00000001) != 1) {
                        emit error(tr("Not a valid IPv4 netmask"),id);
                        return;
                    }

                netmask = 32 - zeros;

            } else {
                emit error(tr("Not an IPv4 netmask"),id);
            }
            output.append(QByteArray::number(netmask));
        } else {
            emit error(tr("Not a valid IPv4 netmask string"),id);
        }
    }
}

bool NetworkMaskIPv4::isTwoWays()
{
    return true;
}

QString NetworkMaskIPv4::help() const
{
    QString help;
    help.append("<p>Translate network mask IPv4 number to an IPv4 string</p><p>Used to save you from having to calculate an unusual netmask value by hand.</p>");
    return help;
}

QString NetworkMaskIPv4::inboundString() const
{
    return tr("number to string");
}

QString NetworkMaskIPv4::outboundString() const
{
    return tr("String to number");
}


