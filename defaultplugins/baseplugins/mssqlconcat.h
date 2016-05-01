/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef MSSQLCONCAT_H
#define MSSQLCONCAT_H

#include "transformabstract.h"

class MSSqlConcat : public TransformAbstract
{
    Q_OBJECT
    public:
        explicit MSSqlConcat() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QString help() const;
};

#endif // MSSQLCONCAT_H
