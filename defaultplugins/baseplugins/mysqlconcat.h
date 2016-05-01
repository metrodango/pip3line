/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MYSQLCONCAT_H
#define MYSQLCONCAT_H

#include "transformabstract.h"

class MysqlConcat : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit MysqlConcat() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // MYSQLCONCAT_H
