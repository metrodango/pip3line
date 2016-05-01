/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef RANDOMCASE_H
#define RANDOMCASE_H

#include "transformabstract.h"

class RandomCase : public TransformAbstract
{
    Q_OBJECT

    public:
        explicit RandomCase();
        ~RandomCase() {}
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        QWidget * requestGui(QWidget * parent);
        static const QString id;    
        QString help() const;
    public slots:
        void reRandomize();

};

#endif // RANDOMCASE_H
