/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MD4_H
#define MD4_H

#include "transformabstract.h"

class Md4 : public TransformAbstract
{
    Q_OBJECT
    
    public:
        explicit Md4() {}
        ~Md4() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;

        QString help() const;

};

#endif // MD4_H
