/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef HMACTRANSFORM_H
#define HMACTRANSFORM_H

#include <transformabstract.h>
#include <QMap>

class HMACTransform : public TransformAbstract
{
        Q_OBJECT
    public:
        static const QMap<int, QString> hashes;
        explicit HMACTransform();
        ~HMACTransform();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        static const QString id;
        int getSelectedHash() const;
        void setSelectedHash(int value);
        QByteArray getKey() const;
        void setKey(const QByteArray &value);
        QString help() const;
    private:
        static const QMap<int, QString> initHash();
        int getBlocksize(int hash);
        int selectedHash;
        QByteArray key;
};

#endif // HMACTRANSFORM_H
