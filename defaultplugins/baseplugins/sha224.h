/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SHA224_H
#define SHA224_H

#include "transformabstract.h"

class Sha224 : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit Sha224();
        ~Sha224();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // SHA224_H
