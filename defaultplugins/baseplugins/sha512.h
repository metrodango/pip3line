/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SHA512_H
#define SHA512_H

#include "transformabstract.h"

class Sha512 : public TransformAbstract
{
    Q_OBJECT
    public:
        Sha512();
        ~Sha512();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // SHA512_H
