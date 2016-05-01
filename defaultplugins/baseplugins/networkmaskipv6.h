/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NETWORKMASKIPV6_H
#define NETWORKMASKIPV6_H

#include <transformabstract.h>

class NetworkMaskIPv6 : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit NetworkMaskIPv6();
        ~NetworkMaskIPv6() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString inboundString() const;
        QString outboundString() const;
        QString help() const;
};

#endif // NETWORKMASKIPV6_H
