/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef NETWORKMASKIPV6_H
#define NETWORKMASKIPV6_H

#include <transformabstract.h>

class NetworkMaskIPv6 : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit NetworkMaskIPv6();
        ~NetworkMaskIPv6() override {}
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString inboundString() const override;
        QString outboundString() const override;
        QString help() const override;
};

#endif // NETWORKMASKIPV6_H
