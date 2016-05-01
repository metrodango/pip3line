/**
Released as open source by Gabriel Caudrelier

Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com

https://github.com/metrodango/pip3line

Released under AGPL see LICENSE for more information
**/

#ifndef IDENTITY_H
#define IDENTITY_H

#include <transformabstract.h>

// Example Transform class which will return the input as the output
// nothing really exciting, more detailed comments in the cpp file.

class QWidget;

class Identity : public TransformAbstract
{
    public:
        Identity();
        ~Identity();
        QString name() const;
        QString description() const;
        void transform(const QByteArray &input, QByteArray &output);
        bool isTwoWays();
        static const QString id;
        QHash<QString, QString> getConfiguration();
        bool setConfiguration(QHash<QString, QString> propertiesList);
        QWidget * requestGui(QWidget * parent);
    private:
        Q_DISABLE_COPY(Identity) // just to avoid stupidity
};

#endif // IDENTITY_H
