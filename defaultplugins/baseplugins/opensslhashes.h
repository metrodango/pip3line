/**
pip3line: easy encoder/decoder and more
Copyright (C) 2012  Gabriel Caudrelier<gabriel.caudrelier@gmail.com>

Pip3line is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Pip3line is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Pip3line.  If not, see <http://www.gnu.org/licenses/>.
**/

#ifndef OPENSSLHASHES_H
#define OPENSSLHASHES_H



#include <QStringList>
#include <QMutex>
#include "transformabstract.h"

class OpenSSLHashes : public TransformAbstract
{
        Q_OBJECT
        
    public:
        static QStringList hashList;
        static const QStringList blacklistHash;
        explicit OpenSSLHashes();
        ~OpenSSLHashes();
        QString name() const;
        QString description() const;
        TransformAbstract::Type getType() const;
        QByteArray transform(const QByteArray &input);
        bool isTwoWays();
        QDomElement getConf(QDomDocument *xmlDoc);
        bool setConf(QDomElement *conf);
        static const QString id;

        QString getHashName();
        bool setHashName(QString name);
    private:
        QString hashName;
        static QMutex initlocker;
};

#endif // OPENSSLHASHES_H

