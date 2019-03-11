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
        ~HMACTransform() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QWidget * requestGui(QWidget * parent) override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        static const QString id;
        int getSelectedHash() const;
        void setSelectedHash(int value);
        QByteArray getKey() const;
        void setKey(const QByteArray &value);
        QString help() const override;
    private:
        static const QMap<int, QString> initHash();
        int getBlocksize(int hash);
        int selectedHash;
        QByteArray key;
};

#endif // HMACTRANSFORM_H
