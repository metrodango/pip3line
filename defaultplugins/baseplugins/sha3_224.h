/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef SHA3_224_H
#define SHA3_224_H

#include "transformabstract.h"

class Sha3_224 : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit Sha3_224();
        ~Sha3_224() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
};

#endif // SHA3_224_H
