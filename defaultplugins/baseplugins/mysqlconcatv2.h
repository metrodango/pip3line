/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MYSQLCONCATV2_H
#define MYSQLCONCATV2_H

#include "transformabstract.h"

class MySqlConcatv2 : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit MySqlConcatv2() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // MYSQLCONCATV2_H
