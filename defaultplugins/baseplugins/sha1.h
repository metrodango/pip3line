/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under BSD 3-Clause see LICENSE for more information
**/

#ifndef SHA1_H
#define SHA1_H

#include "transformabstract.h"

class Sha1 : public TransformAbstract
{
    Q_OBJECT
    
    public:
        explicit Sha1();
        ~Sha1() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
};

#endif // SHA1_H
