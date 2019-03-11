/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef POSTGRESCONCAT_H
#define POSTGRESCONCAT_H

#include "transformabstract.h"

class PostgresConcat : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit PostgresConcat() {}
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        static const QString id;
        QString help() const override;
};

#endif // POSTGRESCONCAT_H
