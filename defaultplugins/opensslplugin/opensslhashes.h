/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef OPENSSLHASHES_H
#define OPENSSLHASHES_H

#include <QStringList>
#include <QMutex>
#include <transformabstract.h>
#include <openssl/evp.h>

class OpenSSLHashes : public TransformAbstract
{
        Q_OBJECT

    public:
        static QStringList hashList;
        explicit OpenSSLHashes(const QString &hashName = QString());
        ~OpenSSLHashes();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        static const QString id;

        QString getHashName();
        bool setHashName(QString name);
        QString help() const;
    private:
        Q_DISABLE_COPY(OpenSSLHashes)
        QString hashName;
};
#endif // OPENSSLHASHES_H
