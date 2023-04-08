/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#include "networkmaskipv6.h"
#include <QHostAddress>
#include <QDebug>

const QString NetworkMaskIPv6::id = "Network mask IPv6";

NetworkMaskIPv6::NetworkMaskIPv6()
{
}

QString NetworkMaskIPv6::name() const
{
    return id;
}


QString NetworkMaskIPv6::description() const
{
    return tr("Translate network mask IPv6 number to a \"readable\" string");
}

void NetworkMaskIPv6::transform(const QByteArray &input, QByteArray &output)
{
    bool ok = false;
    QHostAddress ip;
    Q_IPV6ADDR ipVal;
    if (wayValue == TransformAbstract::INBOUND) {
        int num = input.toInt(&ok,10);
        if (ok && num >= 0 && num < 129 ) {


            int pos = num / 8;

            for (int i = 0; i < pos; i++ ) {
                ipVal[i] = 0xFF;
            }
            ipVal[pos] = ( num % 8 == 0 ? 0x00 : ~(0xFF >> num % 8));
            for (int i = pos + 1; i < 16; i++) {
                ipVal[i] = 0x00;
            }
            ip.setAddress(ipVal);
            output.append(ip.toString().toUtf8());
        } else {
            emit error(tr("Not a valid IPv6 netmask value"),id);
        }
    } else {
        QString val = QString::fromUtf8(input);
        if (ip.setAddress(val)) {

            if (ip.protocol() == QAbstractSocket::IPv6Protocol) {
                int netmask = 0;
                ipVal = ip.toIPv6Address();
                int i = 0;
                for (i = 0; i < 16; i++) {

                    if (ipVal[i] == 0xFF) {
                        netmask += 8;
                    }
                    else
                        break;
                }
                if (i < 16) {
                    int cur = 7;
                    for (; cur >= 0; cur--) {
                        if (((ipVal[i] >> cur) & 0x01) == 1) {
                            netmask++;
                        } else {
                            break;
                        }
                    }
                }
                output.append(QByteArray::number(netmask));
            } else {
                emit error(tr("Not an IPv6 netmask"),id);
            }
        } else {
            emit error(tr("Not a valid IPv6 netmask string"),id);
        }
    }
}

bool NetworkMaskIPv6::isTwoWays()
{
    return true;
}

QString NetworkMaskIPv6::inboundString() const
{
    return tr("number to string");
}

QString NetworkMaskIPv6::outboundString() const
{
    return tr("String to number");
}

QString NetworkMaskIPv6::help() const
{
    QString help;
    help.append("<p>Translate network mask IPv6 number to an IPv6 string</p>");
    return help;
}
