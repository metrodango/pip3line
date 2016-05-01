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
        ~ByteRot();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        QString help() const;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        static const QString id;

        int getRotation() const;
        void setRotation(int rot);

    private:
        int rotation;

};

#endif // BYTEROT_H
