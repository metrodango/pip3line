/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef ORACLECONCAT_H
#define ORACLECONCAT_H

#include "transformabstract.h"

class OracleConcat : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit OracleConcat() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // ORACLECONCAT_H
