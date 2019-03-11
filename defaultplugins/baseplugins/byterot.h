/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef BYTEROT_H
#define BYTEROT_H

#include <transformabstract.h>

class ByteRot : public TransformAbstract
{
        Q_OBJECT
    public:
        explicit ByteRot();
        ~ByteRot() override;
        QString name() const override;
        QString description() const override;
        void transform(const QByteArray &input, QByteArray &output) override;
        bool isTwoWays() override;
        QWidget * requestGui(QWidget * parent) override;
        QString help() const override;
        QHash<QString, QString> getConfiguration() override;
        bool setConfiguration(QHash<QString, QString> propertiesList) override;
        static const QString id;

        int getRotation() const;
        void setRotation(int rot);

    private:
        int rotation;

};

#endif // BYTEROT_H
