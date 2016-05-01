/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/
#ifndef ROTX_H
#define ROTX_H

#include "transformabstract.h"

class Rotx : public TransformAbstract
{
        Q_OBJECT
        
    public:
        enum RotVariant {ROT13 = 0, ROT5 = 1, ROT47 = 2};
        explicit Rotx();
        ~Rotx();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
        static const QString id;
        QString help() const;

        RotVariant getRotVariant();
        void setRotVariant(RotVariant val);
    private:
        RotVariant choosenVariant;
};

#endif // ROTX_H
