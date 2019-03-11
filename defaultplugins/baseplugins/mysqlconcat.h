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
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
};

#endif // MYSQLCONCAT_H
