/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef NETWORKMASK_H
#define NETWORKMASK_H

#include <transformabstract.h>

class NetworkMaskIPv4 : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit NetworkMaskIPv4();
        ~NetworkMaskIPv4() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString inboundString() const;
        QString outboundString() const;
        QString help() const;
};

#endif // NETWORKMASK_H
