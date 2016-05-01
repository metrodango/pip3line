/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef SHA1_H
#define SHA1_H

#include "transformabstract.h"

class Sha1 : public TransformAbstract
{
    Q_OBJECT
    
    public:
        explicit Sha1();
        ~Sha1();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // SHA1_H
