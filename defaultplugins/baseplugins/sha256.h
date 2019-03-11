/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SHA256_H
#define SHA256_H

#include "transformabstract.h"

class Sha256 : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit Sha256();
        ~Sha256() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
};

#endif // SHA256_H
